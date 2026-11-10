#include "features/web_frontend/gui/handlers/database/database_state_handler.h"
#include "features/web_frontend/gui/native_bridge.h"

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
    {
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, userSettingsPath);
        QSettings::setDefaultFormat(QSettings::IniFormat);
    }

    ~ScopedTestSettings() { QSettings::setDefaultFormat(previous_format_); }

private:
    QSettings::Format previous_format_;
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
    void reportsInvalidJson();
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

    const QList<QPair<QString, QVariantMap>> samples = {
        {QStringLiteral("text"), {{QStringLiteral("text_content"), QStringLiteral("bridge test")}}},
        {QStringLiteral("url"),
         {{QStringLiteral("url_content"), QStringLiteral("https://example.com")}}},
        {QStringLiteral("email"),
         {{QStringLiteral("email_address"), QStringLiteral("a@example.com")}}},
        {QStringLiteral("phone"),
         {{QStringLiteral("phone_number"), QStringLiteral("09012345678")}}},
        {QStringLiteral("sms"),
         {{QStringLiteral("sms_phone"), QStringLiteral("09012345678")},
          {QStringLiteral("sms_message"), QStringLiteral("Hello")}}},
        {QStringLiteral("wifi"),
         {{QStringLiteral("wifi_ssid"), QStringLiteral("DevTools")},
          {QStringLiteral("wifi_password"), QStringLiteral("secret")},
          {QStringLiteral("wifi_security"), QStringLiteral("WPA")},
          {QStringLiteral("wifi_hidden"), QStringLiteral("false")}}},
        {QStringLiteral("contact"),
         {{QStringLiteral("contact_name"), QStringLiteral("Alice")},
          {QStringLiteral("contact_phone"), QStringLiteral("09012345678")},
          {QStringLiteral("contact_email"), QStringLiteral("alice@example.com")}}},
        {QStringLiteral("calendar"),
         {{QStringLiteral("cal_summary"), QStringLiteral("Review")},
          {QStringLiteral("cal_start"), QStringLiteral("2026-10-29T10:00")},
          {QStringLiteral("cal_end"), QStringLiteral("2026-10-29T11:00")}}},
        {QStringLiteral("geo"),
         {{QStringLiteral("geo_lat"), QStringLiteral("35.681236")},
          {QStringLiteral("geo_lng"), QStringLiteral("139.767125")}}},
    };
    const QStringList expectedContents = {
        QStringLiteral("bridge test"),
        QStringLiteral("https://example.com"),
        QStringLiteral("mailto:a@example.com"),
        QStringLiteral("tel:09012345678"),
        QStringLiteral("SMSTO:09012345678:Hello"),
        QStringLiteral("WIFI:T:WPA;S:DevTools;P:secret;;"),
        QStringLiteral("BEGIN:VCARD\nVERSION:3.0\nFN:Alice\nTEL:09012345678\n"
                       "EMAIL:alice@example.com\nEND:VCARD"),
        QStringLiteral("BEGIN:VEVENT\nSUMMARY:Review\nDTSTART:20261029T100000\n"
                       "DTEND:20261029T110000\nEND:VEVENT"),
        QStringLiteral("geo:35.681236,139.767125"),
    };

    for (qsizetype index = 0; index < samples.size(); ++index) {
        const int completedBefore = completed.count();
        const int failedBefore = failed.count();
        const QString requestId = QStringLiteral("qr-%1").arg(index);
        bridge.startOperation(
            {{QStringLiteral("contractVersion"), QStringLiteral("2")},
             {QStringLiteral("requestId"), requestId},
             {QStringLiteral("operation"), QStringLiteral("generateQr")},
             {QStringLiteral("payload"),
              QVariantMap{{QStringLiteral("type"), samples.at(index).first},
                          {QStringLiteral("parameters"), samples.at(index).second}}}});
        QTRY_VERIFY_WITH_TIMEOUT(
            completed.count() > completedBefore || failed.count() > failedBefore, 3000);
        if (failed.count() != failedBefore) {
            QFAIL(qPrintable(
                failed.constLast().at(1).toMap().value(QStringLiteral("message")).toString()));
        }

        const QVariantMap result = completed.constLast().at(1).toMap();
        QCOMPARE(result.value(QStringLiteral("content")).toString(), expectedContents.at(index));
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
    QVERIFY(result.contains(QStringLiteral("qtVersion")));
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
            {{QStringLiteral("type"), QStringLiteral("QSQLITE")}}, &result);
    QVERIFY(result.value(QStringLiteral("connected")).toBool());

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
    if (mysqlHost.isEmpty() || mysqlDatabase.isEmpty() || mysqlUsername.isEmpty() ||
        postgresHost.isEmpty() || postgresDatabase.isEmpty() || postgresUsername.isEmpty()) {
        QSKIP("Set DEVTOOLS_TEST_MYSQL_* and DEVTOOLS_TEST_POSTGRES_* to run server integration.");
    }
    QTemporaryDir settingsDirectory;
    QVERIFY(settingsDirectory.isValid());
    ScopedTestSettings scopedSettings(settingsDirectory.path());

    const QList<QVariantMap> connections = {
        {{QStringLiteral("type"), QStringLiteral("QMYSQL")},
         {QStringLiteral("host"), mysqlHost},
         {QStringLiteral("database"), mysqlDatabase},
         {QStringLiteral("username"), mysqlUsername},
         {QStringLiteral("password"), mysqlPassword}},
        {{QStringLiteral("type"), QStringLiteral("QPSQL")},
         {QStringLiteral("host"), postgresHost},
         {QStringLiteral("database"), postgresDatabase},
         {QStringLiteral("username"), postgresUsername},
         {QStringLiteral("password"), postgresPassword}},
    };

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
