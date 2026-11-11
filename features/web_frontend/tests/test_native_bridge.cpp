#include "features/web_frontend/gui/handlers/database/database_state_handler.h"
#include "features/web_frontend/gui/native_bridge.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTimer>
#include <QtTest>

namespace {
QVariantMap formatRequest(const QString &requestId, const QString &text,
                          const QString &indentation = QStringLiteral("2"))
{
    return {{QStringLiteral("contractVersion"), QStringLiteral("2")},
            {QStringLiteral("requestId"), requestId},
            {QStringLiteral("operation"), QStringLiteral("formatJson")},
            {QStringLiteral("payload"), QVariantMap{{QStringLiteral("text"), text},
                                                    {QStringLiteral("indentation"), indentation}}}};
}

QVariantMap databaseRequest(const QString &requestId, const QString &operation,
                            const QVariantMap &payload)
{
    return {{QStringLiteral("contractVersion"), QStringLiteral("2")},
            {QStringLiteral("requestId"), requestId},
            {QStringLiteral("operation"), operation},
            {QStringLiteral("payload"), payload}};
}

struct DatabaseOperationOutcome
{
    bool finished{false};
    bool succeeded{false};
    QVariantMap value;
    QString error;
};

class ScopedTestSettings final
{
public:
    explicit ScopedTestSettings(const QString &userSettingsPath)
        : previous_format_(QSettings::defaultFormat())
        , previous_organization_name_(QCoreApplication::organizationName())
    {
        QCoreApplication::setOrganizationName(QStringLiteral("DevToolsTests"));
        QDir(userSettingsPath).mkpath(QCoreApplication::organizationName());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, userSettingsPath);
        QSettings::setDefaultFormat(QSettings::IniFormat);
    }

    ~ScopedTestSettings()
    {
        QSettings::setDefaultFormat(previous_format_);
        QCoreApplication::setOrganizationName(previous_organization_name_);
    }

private:
    QSettings::Format previous_format_;
    QString previous_organization_name_;
};

DatabaseOperationOutcome executeDatabaseOperation(NativeBridge &bridge, const QString &requestId,
                                                  const QString &operation,
                                                  const QVariantMap &payload)
{
    DatabaseOperationOutcome outcome;
    QEventLoop eventLoop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &eventLoop, &QEventLoop::quit);

    const auto completedConnection = QObject::connect(
        &bridge, &NativeBridge::operationCompleted, &eventLoop,
        [&outcome, &eventLoop, &requestId](const QString &id, const QVariantMap &result) {
            if (id == requestId) {
                outcome.finished = true;
                outcome.succeeded = true;
                outcome.value = result;
                eventLoop.quit();
            }
        });
    const auto failedConnection = QObject::connect(
        &bridge, &NativeBridge::operationFailed, &eventLoop,
        [&outcome, &eventLoop, &requestId](const QString &id, const QVariantMap &error) {
            if (id == requestId) {
                outcome.finished = true;
                outcome.error = error.value(QStringLiteral("message")).toString();
                eventLoop.quit();
            }
        });

    bridge.startOperation(databaseRequest(requestId, operation, payload));
    if (!outcome.finished) {
        timeout.start(5000);
        eventLoop.exec();
    }
    QObject::disconnect(completedConnection);
    QObject::disconnect(failedConnection);
    if (!outcome.finished) {
        outcome.error = QStringLiteral("Timed out waiting for database operation.");
    }
    return outcome;
}
} // namespace

class TestNativeBridge : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void formatsJsonUsingTheNativeParserAndEmitter();
    void comparesTextUsingSharedContractFixture();
    void convertsDataFromSharedContractFixture();
    void rendersMarkdownUsingSharedSafetyFixture();
    void generatesCommandsUsingSharedContractFixture();
    void evaluatesRegexUsingSharedContractFixture();
    void convertsTimestampsUsingSharedContractFixture();
    void reportsInvalidJson();
    void readsTextFileUsingSharedContractFixture();
    void readsLegacyQtSettingsContractFixture();
    void rejectsInvalidUtf8Files();
    void enforcesInputAndVersionLimits();
    void rejectsOperationsOutsideTheRegistry();
    void cancelsBeforeTheWorkerStarts();
    void emitsOnlyOneTerminalEvent();
    void generatesQrThroughTheVersionedBridge();
    void readsDroppedImagesWithoutExposingTheirPaths();
    void loadsImagesInRgbaFormat();
    void readsAboutMetadataAndLicenseResource();
    void keepsSqlTransactionsAcrossQueries();
    void executesDatabaseQueryUsingSharedContractFixture();
    void connectsToLocalMySqlAndPostgreSql();
};

void TestNativeBridge::formatsJsonUsingTheNativeParserAndEmitter()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QVERIFY(completed.isValid());
    QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                  QStringLiteral("/tests/fixtures/tauri-format-json.json"));
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    const QJsonArray cases = QJsonDocument::fromJson(fixture.readAll()).array();
    QVERIFY(!cases.isEmpty());

    for (qsizetype index = 0; index < cases.size(); ++index) {
        const QJsonObject testCase = cases.at(index).toObject();
        const QJsonObject input = testCase.value(QStringLiteral("input")).toObject();
        const QString requestId = QStringLiteral("format-shared-%1").arg(index);
        bridge.startOperation(formatRequest(requestId,
                                            input.value(QStringLiteral("text")).toString(),
                                            input.value(QStringLiteral("indentation")).toString()));
        QTRY_COMPARE_WITH_TIMEOUT(completed.count(), index + 1, 3000);
        const QVariantMap result = completed.at(index).at(1).toMap();
        QCOMPARE(result.value(QStringLiteral("text")).toString(),
                 testCase.value(QStringLiteral("expected")).toString());
    }
}

void TestNativeBridge::reportsInvalidJson()
{
    NativeBridge bridge(nullptr);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QVERIFY(failed.isValid());

    bridge.startOperation(formatRequest(QStringLiteral("invalid-1"), QStringLiteral("{")));
    QTRY_COMPARE_WITH_TIMEOUT(failed.count(), 1, 3000);

    const QVariantMap error = failed.constFirst().at(1).toMap();
    QCOMPARE(error.value(QStringLiteral("code")).toString(), QStringLiteral("INVALID_INPUT"));
    QVERIFY(!error.value(QStringLiteral("message")).toString().isEmpty());
}

void TestNativeBridge::readsLegacyQtSettingsContractFixture()
{
    const QString path = QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                         QStringLiteral("/tests/fixtures/qt-settings-migration.plist");
    QVERIFY(QFile::exists(path));
    const QSettings settings(path, QSettings::NativeFormat);
    QCOMPARE(settings.value(QStringLiteral("language")).toString(), QStringLiteral("ja_JP"));
    QCOMPARE(settings.value(QStringLiteral("general/showSidemenuOnStartup")).toBool(), false);
    QCOMPARE(settings.value(QStringLiteral("window/width")).toInt(), 1280);
    QCOMPARE(settings.value(QStringLiteral("RegexTester/pattern")).toString(),
             QStringLiteral("(?<日本語>猫)"));
    const QStringList history =
        settings.value(QStringLiteral("db_tool/connectionHistory")).toStringList();
    QCOMPARE(history.size(), 2);
    QVERIFY(QJsonDocument::fromJson(history.constFirst().toUtf8())
                .object()
                .contains(QStringLiteral("securityScopedBookmark")));
}

void TestNativeBridge::comparesTextUsingSharedContractFixture()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QVERIFY(completed.isValid());
    QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                  QStringLiteral("/tests/fixtures/tauri-compare-text.json"));
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    const QJsonArray cases = QJsonDocument::fromJson(fixture.readAll()).array();
    QVERIFY(!cases.isEmpty());

    for (qsizetype index = 0; index < cases.size(); ++index) {
        const QJsonObject testCase = cases.at(index).toObject();
        const QVariantMap input = testCase.value(QStringLiteral("input")).toObject().toVariantMap();
        const QString requestId = QStringLiteral("diff-shared-%1").arg(index);
        bridge.startOperation(databaseRequest(requestId, QStringLiteral("compareText"), input));
        QTRY_COMPARE_WITH_TIMEOUT(completed.count(), index + 1, 3000);
        const QVariantList rows =
            completed.at(index).at(1).toMap().value(QStringLiteral("rows")).toList();
        QCOMPARE(QJsonArray::fromVariantList(rows), testCase.value(QStringLiteral("expected"))
                                                        .toObject()
                                                        .value(QStringLiteral("rows"))
                                                        .toArray());
    }
}

void TestNativeBridge::convertsDataFromSharedContractFixture()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                  QStringLiteral("/tests/fixtures/tauri-convert-data.json"));
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    const QJsonArray cases = QJsonDocument::fromJson(fixture.readAll()).array();
    QVERIFY(!cases.isEmpty());

    for (qsizetype index = 0; index < cases.size(); ++index) {
        const int completedBefore = completed.count();
        const int failedBefore = failed.count();
        const QString requestId = QStringLiteral("convert-data-%1").arg(index);
        bridge.startOperation(databaseRequest(
            requestId, QStringLiteral("convertData"),
            cases.at(index).toObject().value(QStringLiteral("input")).toObject().toVariantMap()));
        QTRY_VERIFY_WITH_TIMEOUT(completed.count() > completedBefore, 3000);
        QCOMPARE(failed.count(), failedBefore);
        QCOMPARE(QJsonObject::fromVariantMap(completed.constLast().at(1).toMap()),
                 cases.at(index).toObject().value(QStringLiteral("expected")).toObject());
    }
}

void TestNativeBridge::rendersMarkdownUsingSharedSafetyFixture()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                  QStringLiteral("/tests/fixtures/tauri-markdown.json"));
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    const QJsonObject testCase = QJsonDocument::fromJson(fixture.readAll()).object();
    bridge.startOperation(databaseRequest(
        QStringLiteral("markdown-safety"), QStringLiteral("renderMarkdown"),
        {{QStringLiteral("markdown"), testCase.value(QStringLiteral("markdown")).toString()}}));
    QTRY_COMPARE_WITH_TIMEOUT(completed.count(), 1, 3000);
    const QString html =
        completed.constFirst().at(1).toMap().value(QStringLiteral("html")).toString();
    for (const QJsonValue &value : testCase.value(QStringLiteral("requiredHtml")).toArray()) {
        QVERIFY(html.contains(value.toString()));
    }
    for (const QJsonValue &value : testCase.value(QStringLiteral("forbiddenHtml")).toArray()) {
        QVERIFY2(!html.contains(value.toString(), Qt::CaseInsensitive),
                 qPrintable(value.toString()));
    }
    for (const QJsonValue &value : testCase.value(QStringLiteral("qtExpectedHtml")).toArray()) {
        QVERIFY(html.contains(value.toString(), Qt::CaseInsensitive));
    }
}

void TestNativeBridge::generatesCommandsUsingSharedContractFixture()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                  QStringLiteral("/tests/fixtures/tauri-command.json"));
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    const QJsonArray cases = QJsonDocument::fromJson(fixture.readAll()).array();

    for (qsizetype index = 0; index < cases.size(); ++index) {
        const QJsonObject testCase = cases.at(index).toObject();
        const int completedBefore = completed.count();
        const int failedBefore = failed.count();
        bridge.startOperation(databaseRequest(
            QStringLiteral("command-shared-%1").arg(index), QStringLiteral("commandData"),
            testCase.value(QStringLiteral("input")).toObject().toVariantMap()));
        QTRY_VERIFY_WITH_TIMEOUT(
            completed.count() > completedBefore || failed.count() > failedBefore, 3000);
        if (testCase.contains(QStringLiteral("errorCode"))) {
            QVERIFY(failed.count() > failedBefore);
            QCOMPARE(failed.constLast().at(1).toMap().value(QStringLiteral("code")).toString(),
                     testCase.value(QStringLiteral("errorCode")).toString());
            continue;
        }
        QVERIFY(completed.count() > completedBefore);
        QCOMPARE(QJsonObject::fromVariantMap(completed.constLast().at(1).toMap()),
                 testCase.value(QStringLiteral("expected")).toObject());
    }
}

void TestNativeBridge::evaluatesRegexUsingSharedContractFixture()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                  QStringLiteral("/tests/fixtures/tauri-regex.json"));
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    const QJsonArray cases = QJsonDocument::fromJson(fixture.readAll()).array();
    QVERIFY(!cases.isEmpty());

    for (qsizetype index = 0; index < cases.size(); ++index) {
        const QJsonObject testCase = cases.at(index).toObject();
        const int completedBefore = completed.count();
        const int failedBefore = failed.count();
        const QString requestId = QStringLiteral("regex-shared-%1").arg(index);
        bridge.startOperation(
            databaseRequest(requestId, QStringLiteral("evaluateRegex"),
                            testCase.value(QStringLiteral("input")).toObject().toVariantMap()));
        QTRY_VERIFY_WITH_TIMEOUT(
            completed.count() > completedBefore || failed.count() > failedBefore, 3000);
        if (testCase.contains(QStringLiteral("errorCode"))) {
            QVERIFY(failed.count() > failedBefore);
            QCOMPARE(failed.constLast().at(1).toMap().value(QStringLiteral("code")).toString(),
                     testCase.value(QStringLiteral("errorCode")).toString());
            continue;
        }
        QVERIFY(completed.count() > completedBefore);
        QCOMPARE(QJsonObject::fromVariantMap(completed.constLast().at(1).toMap()),
                 testCase.value(QStringLiteral("expected")).toObject());
    }
}

void TestNativeBridge::convertsTimestampsUsingSharedContractFixture()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                  QStringLiteral("/tests/fixtures/tauri-timestamp.json"));
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    const QJsonArray cases = QJsonDocument::fromJson(fixture.readAll()).array();
    QVERIFY(!cases.isEmpty());

    for (qsizetype index = 0; index < cases.size(); ++index) {
        const QJsonObject testCase = cases.at(index).toObject();
        const QString timeZone = testCase.value(QStringLiteral("timeZone")).toString();
        if (!timeZone.isEmpty() && qEnvironmentVariable("TZ") != timeZone) {
            continue;
        }
        const int completedBefore = completed.count();
        const int failedBefore = failed.count();
        const QString requestId = QStringLiteral("timestamp-shared-%1").arg(index);
        bridge.startOperation(
            databaseRequest(requestId, QStringLiteral("convertTimestamp"),
                            testCase.value(QStringLiteral("input")).toObject().toVariantMap()));
        QTRY_VERIFY_WITH_TIMEOUT(
            completed.count() > completedBefore || failed.count() > failedBefore, 3000);
        if (testCase.contains(QStringLiteral("errorCode"))) {
            QVERIFY(failed.count() > failedBefore);
            QCOMPARE(failed.constLast().at(1).toMap().value(QStringLiteral("code")).toString(),
                     testCase.value(QStringLiteral("errorCode")).toString());
            continue;
        }
        QVERIFY(completed.count() > completedBefore);
        const QVariantMap result = completed.constLast().at(1).toMap();
        const QJsonObject expected = testCase.value(QStringLiteral("expected")).toObject();
        for (const QString &key :
             {QStringLiteral("seconds"), QStringLiteral("milliseconds"), QStringLiteral("utc")}) {
            QCOMPARE(result.value(key).toString(), expected.value(key).toString());
        }
        const QDateTime local = QDateTime::fromString(
            result.value(QStringLiteral("local")).toString(), Qt::ISODateWithMs);
        QVERIFY(local.isValid());
        QCOMPARE(local.toMSecsSinceEpoch(),
                 result.value(QStringLiteral("milliseconds")).toString().toLongLong());
    }
}

void TestNativeBridge::rejectsInvalidUtf8Files()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    QCOMPARE(file.write(QByteArray::fromHex("c328")), 2);
    QVERIFY(file.flush());

    const auto cancellation = std::make_shared<std::atomic_bool>(false);
    const NativeBridge::OperationResult result =
        NativeBridge::readTextFile(file.fileName(), cancellation, [](int, const QString &) {});

    QCOMPARE(result.errorCode, QStringLiteral("INVALID_ENCODING"));
}

void TestNativeBridge::readsTextFileUsingSharedContractFixture()
{
    QFile contractFixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                          QStringLiteral("/tests/fixtures/tauri-open-text-file.json"));
    QVERIFY(contractFixture.open(QIODevice::ReadOnly));
    const QJsonObject contract = QJsonDocument::fromJson(contractFixture.readAll()).object();
    const QJsonObject input = contract.value(QStringLiteral("input")).toObject();
    const QJsonObject expected = contract.value(QStringLiteral("expected")).toObject();

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(input.value(QStringLiteral("fileName")).toString());
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    const QByteArray text = input.value(QStringLiteral("text")).toString().toUtf8();
    QCOMPARE(file.write(text), text.size());
    file.close();

    const auto cancellation = std::make_shared<std::atomic_bool>(false);
    const NativeBridge::OperationResult result =
        NativeBridge::readTextFile(path, cancellation, [](int, const QString &) {});

    QCOMPARE(result.errorCode, QString());
    QCOMPARE(result.value.value(QStringLiteral("fileName")).toString(),
             expected.value(QStringLiteral("fileName")).toString());
    QCOMPARE(result.value.value(QStringLiteral("text")).toString(),
             expected.value(QStringLiteral("text")).toString());
}

void TestNativeBridge::enforcesInputAndVersionLimits()
{
    NativeBridge bridge(nullptr);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QSignalSpy started(&bridge, &NativeBridge::operationStarted);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy progress(&bridge, &NativeBridge::operationProgress);
    QVERIFY(failed.isValid());
    QVERIFY(started.isValid());
    QVERIFY(completed.isValid());
    QVERIFY(progress.isValid());

    bridge.startOperation(formatRequest(
        QStringLiteral("large-1"), QString(NativeBridge::MAX_INPUT_BYTES + 1, QLatin1Char('x'))));
    QCOMPARE(failed.count(), 1);
    QCOMPARE(failed.constFirst().at(1).toMap().value(QStringLiteral("code")).toString(),
             QStringLiteral("SIZE_LIMIT_EXCEEDED"));
    QCOMPARE(started.count(), 1);

    QVariantMap oversizedRequest =
        formatRequest(QStringLiteral("large-envelope-1"), QStringLiteral("{}"));
    oversizedRequest.insert(QStringLiteral("extra"),
                            QString(NativeBridge::MAX_MESSAGE_BYTES, QLatin1Char('x')));
    bridge.startOperation(oversizedRequest);
    QCOMPARE(failed.count(), 2);
    QCOMPARE(failed.constLast().at(0).toString(), QStringLiteral("large-envelope-1"));
    QCOMPARE(failed.constLast().at(1).toMap().value(QStringLiteral("code")).toString(),
             QStringLiteral("SIZE_LIMIT_EXCEEDED"));

    bridge.rejectOversizedRequest(QStringLiteral("oversized-adapter-1"));
    QCOMPARE(failed.count(), 3);
    QCOMPARE(failed.constLast().at(0).toString(), QStringLiteral("oversized-adapter-1"));
    QCOMPARE(failed.constLast().at(1).toMap().value(QStringLiteral("code")).toString(),
             QStringLiteral("SIZE_LIMIT_EXCEEDED"));
    bridge.rejectOversizedRequest(QStringLiteral("invalid/id"));
    QCOMPARE(failed.count(), 3);

    const QVariantMap nearLimitRequest{
        {QStringLiteral("contractVersion"), QStringLiteral("2")},
        {QStringLiteral("requestId"), QStringLiteral("near-limit-1")},
        {QStringLiteral("operation"), QStringLiteral("getAboutInfo")},
        {QStringLiteral("payload"),
         QVariantMap{{QStringLiteral("extra"),
                      QString(NativeBridge::MAX_MESSAGE_BYTES - 1024, QLatin1Char('x'))}}}};
    bridge.startOperation(nearLimitRequest);
    QCOMPARE(started.count(), 2);
    QTRY_COMPARE_WITH_TIMEOUT(completed.count(), 1, 3000);
    const int progressBeforeUnsupportedRequest = progress.count();

    QVariantMap unsupported = formatRequest(QStringLiteral("version-1"), QStringLiteral("{}"));
    unsupported.insert(QStringLiteral("contractVersion"), QStringLiteral("3"));
    bridge.startOperation(unsupported);
    QCOMPARE(failed.count(), 4);
    QCOMPARE(failed.constLast().at(1).toMap().value(QStringLiteral("code")).toString(),
             QStringLiteral("UNSUPPORTED_VERSION"));
    QCOMPARE(started.count(), 2);
    QCOMPARE(progress.count(), progressBeforeUnsupportedRequest);
}

void TestNativeBridge::rejectsOperationsOutsideTheRegistry()
{
    NativeBridge bridge(nullptr);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QSignalSpy started(&bridge, &NativeBridge::operationStarted);
    QVERIFY(failed.isValid());
    QVERIFY(started.isValid());

    QVariantMap request = formatRequest(QStringLiteral("unknown-1"), QStringLiteral("{}"));
    request.insert(QStringLiteral("operation"), QStringLiteral("databaseUnknown"));
    bridge.startOperation(request);

    QCOMPARE(failed.count(), 1);
    QCOMPARE(failed.constFirst().at(1).toMap().value(QStringLiteral("code")).toString(),
             QStringLiteral("INVALID_INPUT"));
    QCOMPARE(started.count(), 0);
}

void TestNativeBridge::cancelsBeforeTheWorkerStarts()
{
    NativeBridge bridge(nullptr);
    QSignalSpy cancelled(&bridge, &NativeBridge::operationCancelled);
    QVERIFY(cancelled.isValid());
    connect(&bridge, &NativeBridge::operationStarted, &bridge,
            [&bridge](const QString &requestId, const QString &) {
                bridge.cancelOperation(requestId);
            });

    bridge.startOperation(formatRequest(QStringLiteral("cancel-1"), QStringLiteral("{}")));

    QCOMPARE(cancelled.count(), 1);
    QCOMPARE(cancelled.constFirst().constFirst().toString(), QStringLiteral("cancel-1"));
}

void TestNativeBridge::emitsOnlyOneTerminalEvent()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QSignalSpy cancelled(&bridge, &NativeBridge::operationCancelled);
    QVERIFY(completed.isValid());
    QVERIFY(failed.isValid());
    QVERIFY(cancelled.isValid());

    bridge.startOperation(formatRequest(QStringLiteral("terminal-1"), QStringLiteral("{}")));

    QTRY_COMPARE_WITH_TIMEOUT(completed.count(), 1, 3000);
    QTest::qWait(20);
    QCOMPARE(failed.count(), 0);
    QCOMPARE(cancelled.count(), 0);
    QCOMPARE(completed.count(), 1);
}

void TestNativeBridge::generatesQrThroughTheVersionedBridge()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QVERIFY(completed.isValid());
    QVERIFY(failed.isValid());

    QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                  QStringLiteral("/tests/fixtures/tauri-qr-content.json"));
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    const QJsonArray cases = QJsonDocument::fromJson(fixture.readAll()).array();
    QVERIFY(!cases.isEmpty());

    for (qsizetype index = 0; index < cases.size(); ++index) {
        const int completedBefore = completed.count();
        const int failedBefore = failed.count();
        const QString requestId = QStringLiteral("qr-%1").arg(index);
        bridge.startOperation({{QStringLiteral("contractVersion"), QStringLiteral("2")},
                               {QStringLiteral("requestId"), requestId},
                               {QStringLiteral("operation"), QStringLiteral("generateQr")},
                               {QStringLiteral("payload"), cases.at(index)
                                                               .toObject()
                                                               .value(QStringLiteral("input"))
                                                               .toObject()
                                                               .toVariantMap()}});
        QTRY_VERIFY_WITH_TIMEOUT(
            completed.count() > completedBefore || failed.count() > failedBefore, 3000);
        if (failed.count() != failedBefore) {
            QFAIL(qPrintable(
                failed.constLast().at(1).toMap().value(QStringLiteral("message")).toString()));
        }

        const QVariantMap result = completed.constLast().at(1).toMap();
        QCOMPARE(result.value(QStringLiteral("content")).toString(),
                 cases.at(index)
                     .toObject()
                     .value(QStringLiteral("expected"))
                     .toObject()
                     .value(QStringLiteral("content"))
                     .toString());
        QVERIFY(result.value(QStringLiteral("imageDataUrl"))
                    .toString()
                    .startsWith(QStringLiteral("data:image/png;base64,")));
    }
}

void TestNativeBridge::readsDroppedImagesWithoutExposingTheirPaths()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString imagePath = directory.filePath(QStringLiteral("dropped.png"));
    QImage image(2, 3, QImage::Format_ARGB32);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath, "PNG"));

    NativeBridge bridge(nullptr);
    QSignalSpy available(&bridge, &NativeBridge::imageDropAvailable);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QVERIFY(available.isValid());
    QVERIFY(completed.isValid());
    QVERIFY(failed.isValid());

    bridge.openDroppedImageFile(imagePath);
    QCOMPARE(available.count(), 1);
    bridge.startOperation({{QStringLiteral("contractVersion"), QStringLiteral("2")},
                           {QStringLiteral("requestId"), QStringLiteral("image-drop-1")},
                           {QStringLiteral("operation"), QStringLiteral("imageTakeDropped")},
                           {QStringLiteral("payload"), QVariantMap{}}});

    QTRY_COMPARE_WITH_TIMEOUT(completed.count(), 1, 3000);
    QCOMPARE(failed.count(), 0);
    const QVariantMap result = completed.constFirst().at(1).toMap();
    QVERIFY(result.value(QStringLiteral("found")).toBool());
    QVERIFY(result.value(QStringLiteral("imageDataUrl"))
                .toString()
                .startsWith(QStringLiteral("data:image/png;base64,")));
    QCOMPARE(result.value(QStringLiteral("fileName")).toString(), QStringLiteral("dropped.png"));
    QCOMPARE(result.value(QStringLiteral("width")).toInt(), 2);
    QCOMPARE(result.value(QStringLiteral("height")).toInt(), 3);
    QVERIFY(!result.contains(QStringLiteral("path")));
    QVERIFY(!result.contains(QStringLiteral("filePath")));
}

void TestNativeBridge::readsAboutMetadataAndLicenseResource()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QVERIFY(completed.isValid());
    QVERIFY(failed.isValid());

    bridge.startOperation({{QStringLiteral("contractVersion"), QStringLiteral("2")},
                           {QStringLiteral("requestId"), QStringLiteral("about-1")},
                           {QStringLiteral("operation"), QStringLiteral("getAboutInfo")},
                           {QStringLiteral("payload"), QVariantMap{}}});

    QTRY_COMPARE_WITH_TIMEOUT(completed.count(), 1, 3000);
    QCOMPARE(failed.count(), 0);
    const QVariantMap result = completed.constFirst().at(1).toMap();
    QVERIFY(result.contains(QStringLiteral("appVersion")));
    QVERIFY(result.contains(QStringLiteral("revision")));
    QVERIFY(result.contains(QStringLiteral("buildEnvironment")));
    QVERIFY(result.contains(QStringLiteral("runtime")));
    QVERIFY(!result.value(QStringLiteral("runtime")).toString().isEmpty());
    QVERIFY(result.value(QStringLiteral("copyright"))
                .toString()
                .contains(QStringLiteral("Lotus&Company")));
    QVERIFY(result.contains(QStringLiteral("licenseText")));
    QVERIFY(!result.value(QStringLiteral("licenseText")).toString().trimmed().isEmpty());
}

void TestNativeBridge::loadsImagesInRgbaFormat()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString imagePath = directory.filePath(QStringLiteral("rgb.png"));
    QImage source(2, 2, QImage::Format_RGB32);
    source.fill(Qt::red);
    QVERIFY(source.save(imagePath, "PNG"));

    const auto cancellation = std::make_shared<std::atomic_bool>(false);
    const NativeBridge::OperationResult result =
        NativeBridge::readImageFile(imagePath, cancellation, [](int, const QString &) {});

    QVERIFY(result.errorCode.isEmpty());
    const QImage loaded = result.value.value(QStringLiteral("__image")).value<QImage>();
    QCOMPARE(loaded.format(), QImage::Format_RGBA8888);
}

void TestNativeBridge::keepsSqlTransactionsAcrossQueries()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ScopedTestSettings scopedSettings(directory.filePath(QStringLiteral("settings")));
    const QString databasePath = directory.filePath(QStringLiteral("transaction.sqlite"));
    const QString setupConnectionName = QStringLiteral("test-transaction-setup");
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), setupConnectionName);
        database.setDatabaseName(databasePath);
        QVERIFY(database.open());
        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral("CREATE TABLE items(value INTEGER NOT NULL)")));
        QVERIFY(query.exec(QStringLiteral("INSERT INTO items VALUES (0)")));
        database.close();
    }
    QSqlDatabase::removeDatabase(setupConnectionName);

    NativeBridge bridge(nullptr);
    QVERIFY(bridge.database_state_->selectFile(databasePath));
    {
        QSettings settings;
        const QJsonObject savedConnection{
            {QStringLiteral("type"), QStringLiteral("QSQLITE")},
            {QStringLiteral("host"), QString()},
            {QStringLiteral("database"), bridge.database_state_->selectedFilePath()},
            {QStringLiteral("username"), QString()},
            {QStringLiteral("displayName"), QStringLiteral("SQLite: transaction.sqlite")}};
        settings.setValue(QStringLiteral("db_tool/connectionHistory"),
                          QStringList{QString::fromUtf8(
                              QJsonDocument(savedConnection).toJson(QJsonDocument::Compact))});
        settings.sync();
        QVERIFY(settings.status() == QSettings::NoError);
    }

    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QSignalSpy failed(&bridge, &NativeBridge::operationFailed);
    QVERIFY(completed.isValid());
    QVERIFY(failed.isValid());

    const auto execute = [&](const QString &requestId, const QString &operation,
                             const QVariantMap &payload, QVariantMap *result) {
        const int completedBefore = completed.count();
        const int failedBefore = failed.count();
        bridge.startOperation(databaseRequest(requestId, operation, payload));
        QTRY_VERIFY_WITH_TIMEOUT(
            completed.count() > completedBefore || failed.count() > failedBefore, 3000);
        if (failed.count() > failedBefore) {
            QFAIL(qPrintable(
                failed.constLast().at(1).toMap().value(QStringLiteral("message")).toString()));
            return;
        }
        *result = completed.constLast().at(1).toMap();
    };

    QVariantMap result;
    execute(QStringLiteral("db-connect"), QStringLiteral("databaseConnect"),
            {{QStringLiteral("type"), QStringLiteral("sqlite")}}, &result);
    QVERIFY(result.value(QStringLiteral("connected")).toBool());
    execute(QStringLiteral("db-history"), QStringLiteral("databaseHistory"), {}, &result);
    QCOMPARE(result.value(QStringLiteral("connectedType")).toString(), QStringLiteral("sqlite"));
    const QVariantList history = result.value(QStringLiteral("history")).toList();
    QVERIFY(!history.isEmpty());
    QCOMPARE(history.constFirst().toMap().value(QStringLiteral("type")).toString(),
             QStringLiteral("sqlite"));

    execute(QStringLiteral("db-begin"), QStringLiteral("databaseQuery"),
            {{QStringLiteral("sql"), QStringLiteral("BEGIN;")}}, &result);
    execute(QStringLiteral("db-update"), QStringLiteral("databaseQuery"),
            {{QStringLiteral("sql"), QStringLiteral("UPDATE items SET value = 1;")}}, &result);
    execute(QStringLiteral("db-rollback"), QStringLiteral("databaseQuery"),
            {{QStringLiteral("sql"), QStringLiteral("ROLLBACK;")}}, &result);
    execute(QStringLiteral("db-select"), QStringLiteral("databaseQuery"),
            {{QStringLiteral("sql"), QStringLiteral("SELECT value FROM items;")}}, &result);

    const QVariantList rows = result.value(QStringLiteral("rows")).toList();
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows.constFirst().toList().constFirst().toString(), QStringLiteral("0"));
}

void TestNativeBridge::executesDatabaseQueryUsingSharedContractFixture()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ScopedTestSettings scopedSettings(directory.filePath(QStringLiteral("settings")));
    const QString databasePath = directory.filePath(QStringLiteral("fixture.sqlite"));
    QFile databaseFile(databasePath);
    QVERIFY(databaseFile.open(QIODevice::WriteOnly));
    databaseFile.close();

    NativeBridge bridge(nullptr);
    QVERIFY(bridge.database_state_->selectFile(databasePath));
    auto outcome = executeDatabaseOperation(bridge, QStringLiteral("fixture-connect"),
                                            QStringLiteral("databaseConnect"),
                                            {{QStringLiteral("type"), QStringLiteral("sqlite")}});
    QVERIFY2(outcome.succeeded, qPrintable(outcome.error));

    QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                  QStringLiteral("/tests/fixtures/tauri-sqlite-query.json"));
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    const QJsonArray cases = QJsonDocument::fromJson(fixture.readAll()).array();
    QVERIFY(!cases.isEmpty());
    for (qsizetype index = 0; index < cases.size(); ++index) {
        const QJsonObject testCase = cases.at(index).toObject();
        const QString requestId = QStringLiteral("fixture-query-%1").arg(index);
        outcome = executeDatabaseOperation(
            bridge, requestId, QStringLiteral("databaseQuery"),
            {{QStringLiteral("sql"), testCase.value(QStringLiteral("input"))
                                         .toObject()
                                         .value(QStringLiteral("sql"))
                                         .toString()}});
        QVERIFY2(outcome.succeeded, qPrintable(outcome.error));
        QCOMPARE(QJsonObject::fromVariantMap(outcome.value),
                 testCase.value(QStringLiteral("expected")).toObject());
    }

    outcome = executeDatabaseOperation(bridge, QStringLiteral("fixture-disconnect"),
                                       QStringLiteral("databaseDisconnect"), {});
    QVERIFY2(outcome.succeeded, qPrintable(outcome.error));
}

void TestNativeBridge::connectsToLocalMySqlAndPostgreSql()
{
    const QString mysqlHost = qEnvironmentVariable("DEVTOOLS_TEST_MYSQL_HOST");
    const QString mysqlDatabase = qEnvironmentVariable("DEVTOOLS_TEST_MYSQL_DATABASE");
    const QString mysqlUsername = qEnvironmentVariable("DEVTOOLS_TEST_MYSQL_USERNAME");
    const QString mysqlPassword = qEnvironmentVariable("DEVTOOLS_TEST_MYSQL_PASSWORD");
    const QString postgresHost = qEnvironmentVariable("DEVTOOLS_TEST_POSTGRES_HOST");
    const QString postgresDatabase = qEnvironmentVariable("DEVTOOLS_TEST_POSTGRES_DATABASE");
    const QString postgresUsername = qEnvironmentVariable("DEVTOOLS_TEST_POSTGRES_USERNAME");
    const QString postgresPassword = qEnvironmentVariable("DEVTOOLS_TEST_POSTGRES_PASSWORD");
    QList<QVariantMap> connections;
    if (!mysqlHost.isEmpty() && !mysqlDatabase.isEmpty() && !mysqlUsername.isEmpty()) {
        connections.append({{QStringLiteral("type"), QStringLiteral("mysql")},
                            {QStringLiteral("host"), mysqlHost},
                            {QStringLiteral("database"), mysqlDatabase},
                            {QStringLiteral("username"), mysqlUsername},
                            {QStringLiteral("password"), mysqlPassword}});
    }
    if (!postgresHost.isEmpty() && !postgresDatabase.isEmpty() && !postgresUsername.isEmpty()) {
        connections.append({{QStringLiteral("type"), QStringLiteral("postgres")},
                            {QStringLiteral("host"), postgresHost},
                            {QStringLiteral("database"), postgresDatabase},
                            {QStringLiteral("username"), postgresUsername},
                            {QStringLiteral("password"), postgresPassword}});
    }
    if (connections.isEmpty()) {
        QSKIP("Set one DEVTOOLS_TEST_MYSQL_* or DEVTOOLS_TEST_POSTGRES_* connection.");
    }
    QTemporaryDir settingsDirectory;
    QVERIFY(settingsDirectory.isValid());
    ScopedTestSettings scopedSettings(settingsDirectory.path());

    NativeBridge bridge(nullptr);
    quint64 requestNumber = 0;
    for (const QVariantMap &connection : connections) {
        const QString driver = connection.value(QStringLiteral("type")).toString();
        const auto execute = [&bridge, &requestNumber, &driver](const QString &operation,
                                                                const QVariantMap &payload) {
            const QString requestId =
                QStringLiteral("server-db-%1-%2").arg(driver).arg(++requestNumber);
            return executeDatabaseOperation(bridge, requestId, operation, payload);
        };

        DatabaseOperationOutcome outcome = execute(QStringLiteral("databaseConnect"), connection);
        QVERIFY2(outcome.succeeded, qPrintable(outcome.error));
        QVERIFY(outcome.value.value(QStringLiteral("connected")).toBool());

        outcome = execute(QStringLiteral("databaseQuery"),
                          {{QStringLiteral("sql"), QStringLiteral("SELECT VERSION()")}});
        QVERIFY2(outcome.succeeded, qPrintable(outcome.error));
        const QVariantList versionRows = outcome.value.value(QStringLiteral("rows")).toList();
        QVERIFY(!versionRows.isEmpty());
        QVERIFY(!versionRows.constFirst().toList().constFirst().toString().isEmpty());

        const QStringList statements = {
            QStringLiteral(
                "CREATE TEMPORARY TABLE devtools_review_session (value INTEGER NOT NULL)"),
            QStringLiteral("INSERT INTO devtools_review_session VALUES (0)"),
            QStringLiteral("BEGIN"),
            QStringLiteral("UPDATE devtools_review_session SET value = 1"),
            QStringLiteral("ROLLBACK"),
            QStringLiteral("SELECT value FROM devtools_review_session"),
        };
        for (qsizetype index = 0; index < statements.size(); ++index) {
            outcome = execute(QStringLiteral("databaseQuery"),
                              {{QStringLiteral("sql"), statements.at(index)}});
            QVERIFY2(outcome.succeeded, qPrintable(outcome.error));
            if (index == statements.size() - 1) {
                const QVariantList rows = outcome.value.value(QStringLiteral("rows")).toList();
                QCOMPARE(rows.size(), 1);
                QCOMPARE(rows.constFirst().toList().constFirst().toString(), QStringLiteral("0"));
            }
        }

        outcome = execute(QStringLiteral("databaseDisconnect"), {});
        QVERIFY2(outcome.succeeded, qPrintable(outcome.error));
        QVERIFY(outcome.value.value(QStringLiteral("disconnected")).toBool());
    }
}

QTEST_GUILESS_MAIN(TestNativeBridge)

#include "test_native_bridge.moc"
