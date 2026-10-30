#include "features/web_frontend/gui/native_bridge.h"

#include <QImage>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
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
    void readsAboutMetadataAndLicenseResource();
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
    unsupported.insert(QStringLiteral("contractVersion"), QStringLiteral("3"));
    bridge.startOperation(unsupported);
    QCOMPARE(failed.count(), 2);
    QCOMPARE(failed.constLast().at(1).toMap().value(QStringLiteral("code")).toString(),
             QStringLiteral("UNSUPPORTED_VERSION"));
    QCOMPARE(started.count(), 1);
    QCOMPARE(progress.count(), 0);
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

QTEST_GUILESS_MAIN(TestNativeBridge)

#include "test_native_bridge.moc"
