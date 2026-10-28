#include "features/web_frontend/gui/native_bridge.h"

#include <QSignalSpy>
#include <QTemporaryFile>
#include <QtTest>

namespace {
QVariantMap formatRequest(const QString &requestId, const QString &text,
                          const QString &indentation = QStringLiteral("2"))
{
    return {{QStringLiteral("contractVersion"), QStringLiteral("1")},
            {QStringLiteral("requestId"), requestId},
            {QStringLiteral("operation"), QStringLiteral("formatJson")},
            {QStringLiteral("payload"), QVariantMap{{QStringLiteral("text"), text},
                                                    {QStringLiteral("indentation"), indentation}}}};
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
    void cancelsBeforeTheWorkerStarts();
};

void TestNativeBridge::formatsJsonUsingTheNativeParserAndEmitter()
{
    NativeBridge bridge(nullptr);
    QSignalSpy completed(&bridge, &NativeBridge::operationCompleted);
    QVERIFY(completed.isValid());

    bridge.startOperation(formatRequest(QStringLiteral("format-1"), QStringLiteral("{\"a\":1}")));
    QTRY_COMPARE_WITH_TIMEOUT(completed.count(), 1, 3000);

    const QVariantMap result = completed.constFirst().at(1).toMap();
    QCOMPARE(result.value(QStringLiteral("text")).toString(), QStringLiteral("{\n  \"a\": 1\n}\n"));
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
    QSignalSpy progress(&bridge, &NativeBridge::operationProgress);
    QVERIFY(failed.isValid());
    QVERIFY(started.isValid());
    QVERIFY(progress.isValid());

    bridge.startOperation(formatRequest(
        QStringLiteral("large-1"), QString(NativeBridge::MAX_INPUT_BYTES + 1, QLatin1Char('x'))));
    QCOMPARE(failed.count(), 1);
    QCOMPARE(failed.constFirst().at(1).toMap().value(QStringLiteral("code")).toString(),
             QStringLiteral("SIZE_LIMIT_EXCEEDED"));
    QCOMPARE(started.count(), 1);

    QVariantMap unsupported = formatRequest(QStringLiteral("version-1"), QStringLiteral("{}"));
    unsupported.insert(QStringLiteral("contractVersion"), QStringLiteral("2"));
    bridge.startOperation(unsupported);
    QCOMPARE(failed.count(), 2);
    QCOMPARE(failed.constLast().at(1).toMap().value(QStringLiteral("code")).toString(),
             QStringLiteral("UNSUPPORTED_VERSION"));
    QCOMPARE(started.count(), 1);
    QCOMPARE(progress.count(), 0);
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

QTEST_GUILESS_MAIN(TestNativeBridge)

#include "test_native_bridge.moc"
