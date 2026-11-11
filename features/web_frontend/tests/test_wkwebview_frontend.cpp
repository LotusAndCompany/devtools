#include "features/web_frontend/gui/native_bridge.h"
#include "features/web_frontend/gui/wk_webview_adapter.h"

#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

#include <memory>

class TestWKWebViewFrontend : public QObject
{
    Q_OBJECT

private slots:
    void loadsBundledFrontendAndRunsBridgeRequest();
    void runsSharedFeatureFixturesThroughWebViewBridge();
    void rendersEveryToolRoute();
    void blocksExternalMainFrameNavigation();
    void routesSupportedNativeFileDrops();
    void correlatesOversizedTrustedMessages();

private:
    bool evaluateJavaScript(WKWebViewAdapter *adapter, const QString &script, QVariant *result);
    bool sendOperation(WKWebViewAdapter *adapter, const QString &requestId,
                       const QString &operation, const QJsonObject &payload, QJsonObject *event);
};

namespace {
struct JavaScriptEvaluation
{
    QEventLoop loop;
    QVariant result;
    bool finished = false;
};
} // namespace

bool TestWKWebViewFrontend::evaluateJavaScript(WKWebViewAdapter *adapter, const QString &script,
                                               QVariant *result)
{
    const auto evaluation = std::make_shared<JavaScriptEvaluation>();
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &evaluation->loop, &QEventLoop::quit);
    adapter->evaluateJavaScriptForTesting(script, [evaluation](const QVariant &value) {
        evaluation->result = value;
        evaluation->finished = true;
        evaluation->loop.quit();
    });
    timeout.start(5000);
    if (!evaluation->finished) {
        evaluation->loop.exec();
    }
    timeout.stop();
    if (evaluation->finished) {
        *result = evaluation->result;
    }
    return evaluation->finished;
}

bool TestWKWebViewFrontend::sendOperation(WKWebViewAdapter *adapter, const QString &requestId,
                                          const QString &operation, const QJsonObject &payload,
                                          QJsonObject *event)
{
    QJsonObject request{{QStringLiteral("contractVersion"), QStringLiteral("2")},
                        {QStringLiteral("requestId"), requestId},
                        {QStringLiteral("operation"), operation},
                        {QStringLiteral("payload"), payload}};
    const QString message =
        QStringLiteral("window.webkit.messageHandlers.devtoolsNativeBridge.postMessage(") +
        QString::fromUtf8(
            QJsonDocument(QJsonObject{{QStringLiteral("type"), QStringLiteral("startOperation")},
                                      {QStringLiteral("request"), request}})
                .toJson(QJsonDocument::Compact)) +
        QStringLiteral("); true");
    QVariant accepted;
    if (!evaluateJavaScript(adapter, message, &accepted) || !accepted.toBool()) {
        return false;
    }

    const QString eventScript =
        QStringLiteral("JSON.stringify(window.__wkFixtureEvents?.['%1'] ?? null)").arg(requestId);
    for (int attempt = 0; attempt < 100; ++attempt) {
        QVariant value;
        if (!evaluateJavaScript(adapter, eventScript, &value)) {
            return false;
        }
        const QJsonDocument document = QJsonDocument::fromJson(value.toString().toUtf8());
        if (document.isObject()) {
            *event = document.object();
            return true;
        }
        QTest::qWait(50);
    }
    return false;
}

void TestWKWebViewFrontend::loadsBundledFrontendAndRunsBridgeRequest()
{
    WKWebViewAdapter adapter;
    adapter.resize(900, 700);
    QSignalSpy started(adapter.nativeBridge(), &NativeBridge::operationStarted);
    QSignalSpy loaded(&adapter, &WKWebViewAdapter::frontendLoadFinished);
    adapter.show();
    QTRY_VERIFY_WITH_TIMEOUT(loaded.count() > 0, 15000);
    QVERIFY(loaded.first().at(0).toBool());
    const auto requestedSettings = [&started]() {
        for (const QList<QVariant> &arguments : started) {
            if (arguments.at(1).toString() == QStringLiteral("getSettings")) {
                return true;
            }
        }
        return false;
    };
    QTRY_VERIFY_WITH_TIMEOUT(requestedSettings(), 15000);

    QSignalSpy completed(adapter.nativeBridge(), &NativeBridge::operationCompleted);
    QVariant listenerInstalled;
    QVERIFY(evaluateJavaScript(
        &adapter,
        QStringLiteral("window.__wkNativeEvent=null; window.addEventListener("
                       "'devtools-native-event',e=>{if(e.detail.requestId==='wkwebview_get_about') "
                       "window.__wkNativeEvent=e.detail}); true"),
        &listenerInstalled));
    bool scriptReturned = false;
    adapter.evaluateJavaScriptForTesting(
        QStringLiteral("window.webkit.messageHandlers.devtoolsNativeBridge.postMessage({"
                       "type:'startOperation',request:{contractVersion:'2',"
                       "requestId:'wkwebview_get_about',operation:'getAboutInfo',payload:{}}});"
                       "true"),
        [&scriptReturned](const QVariant &result) { scriptReturned = result.toBool(); });

    QTRY_VERIFY_WITH_TIMEOUT(scriptReturned, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(completed.count(), 1, 15000);
    QCOMPARE(completed.first().at(0).toString(), QStringLiteral("wkwebview_get_about"));

    QTest::qWait(100);
    QVariant eventValue;
    QVERIFY(evaluateJavaScript(
        &adapter, QStringLiteral("JSON.stringify(window.__wkNativeEvent ?? null)"), &eventValue));
    const QJsonObject event = QJsonDocument::fromJson(eventValue.toString().toUtf8()).object();
    QCOMPARE(event.value(QStringLiteral("type")).toString(), QStringLiteral("completed"));
    QCOMPARE(event.value(QStringLiteral("requestId")).toString(),
             QStringLiteral("wkwebview_get_about"));
}

void TestWKWebViewFrontend::runsSharedFeatureFixturesThroughWebViewBridge()
{
    WKWebViewAdapter adapter;
    adapter.resize(900, 700);
    adapter.show();
    QSignalSpy loaded(&adapter, &WKWebViewAdapter::frontendLoadFinished);
    QTRY_VERIFY_WITH_TIMEOUT(loaded.count() > 0, 15000);
    QVERIFY(loaded.first().at(0).toBool());

    QVariant listenerInstalled;
    QVERIFY(evaluateJavaScript(
        &adapter,
        QStringLiteral(
            "window.__wkFixtureEvents = Object.create(null); "
            "window.addEventListener('devtools-native-event', event => { "
            "const detail = event.detail; "
            "if (detail?.requestId && ['completed','failed','cancelled'].includes(detail.type)) "
            "window.__wkFixtureEvents[detail.requestId] = detail; }); true"),
        &listenerInstalled));

    int requestNumber = 0;
    const auto runCases = [&](const QString &fixtureName, const QString &operation,
                              bool expectedIsString = false) {
        QFile fixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) + QStringLiteral("/tests/fixtures/") +
                      fixtureName);
        QVERIFY2(fixture.open(QIODevice::ReadOnly), qPrintable(fixtureName));
        const QJsonArray cases = QJsonDocument::fromJson(fixture.readAll()).array();
        QVERIFY2(!cases.isEmpty(), qPrintable(fixtureName));

        for (const QJsonValue &value : cases) {
            const QJsonObject testCase = value.toObject();
            const QString timeZone = testCase.value(QStringLiteral("timeZone")).toString();
            if (!timeZone.isEmpty() && qEnvironmentVariable("TZ") != timeZone) {
                continue;
            }
            const QString requestId = QStringLiteral("wkFixture%1").arg(requestNumber++);
            QJsonObject event;
            const bool completed =
                sendOperation(&adapter, requestId, operation,
                              testCase.value(QStringLiteral("input")).toObject(), &event);
            QVERIFY2(completed, qPrintable(fixtureName + QStringLiteral(": bridge timed out")));
            if (testCase.contains(QStringLiteral("errorCode"))) {
                QCOMPARE(event.value(QStringLiteral("type")).toString(), QStringLiteral("failed"));
                QCOMPARE(event.value(QStringLiteral("error"))
                             .toObject()
                             .value(QStringLiteral("code"))
                             .toString(),
                         testCase.value(QStringLiteral("errorCode")).toString());
                continue;
            }

            QCOMPARE(event.value(QStringLiteral("type")).toString(), QStringLiteral("completed"));
            QJsonObject expected;
            if (expectedIsString) {
                expected.insert(QStringLiteral("text"), testCase.value(QStringLiteral("expected")));
            } else {
                expected = testCase.value(QStringLiteral("expected")).toObject();
            }
            const QJsonObject result = event.value(QStringLiteral("result")).toObject();
            for (auto it = expected.constBegin(); it != expected.constEnd(); ++it) {
                QCOMPARE(result.value(it.key()), it.value());
            }
        }
    };

    runCases(QStringLiteral("tauri-format-json.json"), QStringLiteral("formatJson"), true);
    runCases(QStringLiteral("tauri-convert-data.json"), QStringLiteral("convertData"));
    runCases(QStringLiteral("tauri-compare-text.json"), QStringLiteral("compareText"));
    runCases(QStringLiteral("tauri-timestamp.json"), QStringLiteral("convertTimestamp"));
    runCases(QStringLiteral("tauri-command.json"), QStringLiteral("commandData"));
    runCases(QStringLiteral("tauri-regex.json"), QStringLiteral("evaluateRegex"));
    runCases(QStringLiteral("tauri-qr-content.json"), QStringLiteral("generateQr"));

    QFile markdownFixture(QStringLiteral(DEVTOOLS_SOURCE_DIR) +
                          QStringLiteral("/tests/fixtures/tauri-markdown.json"));
    QVERIFY(markdownFixture.open(QIODevice::ReadOnly));
    const QJsonObject markdown = QJsonDocument::fromJson(markdownFixture.readAll()).object();
    const QString markdownId = QStringLiteral("wkFixtureMarkdown");
    QJsonObject markdownEvent;
    QVERIFY(sendOperation(
        &adapter, markdownId, QStringLiteral("renderMarkdown"),
        {{QStringLiteral("markdown"), markdown.value(QStringLiteral("markdown")).toString()}},
        &markdownEvent));
    QCOMPARE(markdownEvent.value(QStringLiteral("type")).toString(), QStringLiteral("completed"));
    const QString html = markdownEvent.value(QStringLiteral("result"))
                             .toObject()
                             .value(QStringLiteral("html"))
                             .toString();
    for (const QJsonValue &required : markdown.value(QStringLiteral("requiredHtml")).toArray()) {
        QVERIFY(html.contains(required.toString()));
    }
    for (const QJsonValue &forbidden : markdown.value(QStringLiteral("forbiddenHtml")).toArray()) {
        QVERIFY(!html.contains(forbidden.toString(), Qt::CaseInsensitive));
    }
    for (const QJsonValue &expected : markdown.value(QStringLiteral("qtExpectedHtml")).toArray()) {
        QVERIFY(html.contains(expected.toString(), Qt::CaseInsensitive));
    }
}

void TestWKWebViewFrontend::correlatesOversizedTrustedMessages()
{
    WKWebViewAdapter adapter;
    QSignalSpy failed(adapter.nativeBridge(), &NativeBridge::operationFailed);
    QVERIFY(failed.isValid());

    const QString sourceUrl = adapter.development_server_url_.isEmpty()
                                  ? QUrl::fromLocalFile(adapter.index_file_path_).toString()
                                  : adapter.development_server_url_ + QStringLiteral("/");
    adapter.handleOversizedScriptMessage(sourceUrl, true, QStringLiteral("oversized-1"));
    QCOMPARE(failed.count(), 1);
    QCOMPARE(failed.constFirst().at(0).toString(), QStringLiteral("oversized-1"));
    QCOMPARE(failed.constFirst().at(1).toMap().value(QStringLiteral("code")).toString(),
             QStringLiteral("SIZE_LIMIT_EXCEEDED"));

    adapter.handleOversizedScriptMessage(sourceUrl, true, QStringLiteral("invalid/id"));
    adapter.handleOversizedScriptMessage(QStringLiteral("https://example.com"), true,
                                         QStringLiteral("untrusted-1"));
    adapter.handleOversizedScriptMessage(sourceUrl, false, QStringLiteral("subframe-1"));
    QCOMPARE(failed.count(), 1);
}

void TestWKWebViewFrontend::rendersEveryToolRoute()
{
    WKWebViewAdapter adapter;
    adapter.resize(1100, 760);
    adapter.show();

    QSignalSpy loaded(&adapter, &WKWebViewAdapter::frontendLoadFinished);
    QTRY_VERIFY_WITH_TIMEOUT(loaded.count() > 0, 15000);
    QVERIFY(loaded.first().at(0).toBool());

    const QString snapshotScript = QStringLiteral(
        "JSON.stringify({toolCount: document.querySelectorAll('.app-sidebar-nav "
        "[data-slot=sidebar-menu-button]').length - 1, "
        "main: Boolean(document.querySelector('.app-main')), "
        "mounted: Boolean(document.querySelector('#root > *'))})");
    QJsonObject snapshot;
    bool rendered = false;
    for (int attempt = 0; attempt < 50; ++attempt) {
        QVariant value;
        QVERIFY(evaluateJavaScript(&adapter, snapshotScript, &value));
        snapshot = QJsonDocument::fromJson(value.toString().toUtf8()).object();
        if (snapshot.value(QStringLiteral("main")).toBool() &&
            snapshot.value(QStringLiteral("mounted")).toBool() &&
            snapshot.value(QStringLiteral("toolCount")).toInt() == 11) {
            rendered = true;
            break;
        }
        QTest::qWait(100);
    }
    QVERIFY2(rendered, "The WKWebView frontend did not render all 11 tool routes.");

    QVariant logoLoaded;
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(
            &adapter, QStringLiteral("document.querySelector('.brand-logo')?.naturalWidth > 0"),
            &logoLoaded) &&
            logoLoaded.toBool(),
        5000);

    for (int index = 0; index < 11; ++index) {
        QVariant expectedValue;
        QVERIFY(evaluateJavaScript(
            &adapter,
            QStringLiteral(
                "document.querySelectorAll('.app-sidebar-nav "
                "[data-slot=sidebar-menu-button]')[%1]?.lastElementChild?.textContent ?? ''")
                .arg(index + 1),
            &expectedValue));
        QString expectedHeading = expectedValue.toString().trimmed();
        if (index == 6) {
            expectedHeading = expectedHeading == QStringLiteral("QRコード")
                                  ? QStringLiteral("QRコード生成")
                                  : QStringLiteral("QR Code Generation");
        }
        QVERIFY(!expectedHeading.isEmpty());

        QVariant clicked;
        QVERIFY(
            evaluateJavaScript(&adapter,
                               QStringLiteral("document.querySelectorAll('.app-sidebar-nav "
                                              "[data-slot=sidebar-menu-button]')[%1].click(); true")
                                   .arg(index + 1),
                               &clicked));
        QVERIFY(clicked.toBool());

        QString headingText;
        for (int attempt = 0; attempt < 50; ++attempt) {
            QVariant heading;
            QVERIFY(evaluateJavaScript(
                &adapter,
                QStringLiteral("document.querySelector('.page-host:not([hidden]) "
                               ".page-heading h1')?.textContent ?? ''"),
                &heading));
            headingText = heading.toString().trimmed();
            if (headingText == expectedHeading) {
                break;
            }
            QTest::qWait(100);
        }
        QCOMPARE(headingText, expectedHeading);

        if (index == 7) {
            bool previewRendered = false;
            for (int attempt = 0; attempt < 100; ++attempt) {
                QVariant previewValue;
                QVERIFY(evaluateJavaScript(
                    &adapter,
                    QStringLiteral(
                        "JSON.stringify({heading:document.querySelector('.markdown-columns iframe')"
                        "?.contentDocument?.querySelector('h1')?.textContent ?? '',"
                        "body:document.querySelector('.markdown-columns iframe')?.contentDocument"
                        "?.body?.innerText ?? ''})"),
                    &previewValue));
                const QJsonObject preview =
                    QJsonDocument::fromJson(previewValue.toString().toUtf8()).object();
                if (preview.value(QStringLiteral("heading")).toString() ==
                        QStringLiteral("Markdown") &&
                    preview.value(QStringLiteral("body"))
                        .toString()
                        .contains(QStringLiteral("Write here to preview."))) {
                    previewRendered = true;
                    break;
                }
                QTest::qWait(100);
            }
            QVERIFY2(previewRendered,
                     "The sandboxed Markdown srcdoc preview did not render in WKWebView.");
        }
    }
}

void TestWKWebViewFrontend::blocksExternalMainFrameNavigation()
{
    WKWebViewAdapter adapter;
    adapter.resize(900, 700);
    adapter.show();

    QSignalSpy loaded(&adapter, &WKWebViewAdapter::frontendLoadFinished);
    QTRY_VERIFY_WITH_TIMEOUT(loaded.count() > 0, 15000);
    QVERIFY(loaded.first().at(0).toBool());

    QSignalSpy rejected(&adapter, &WKWebViewAdapter::navigationRejected);
    adapter.evaluateJavaScriptForTesting(
        QStringLiteral("window.location.href='https://example.com/'; true"),
        [](const QVariant &) {});
    QTRY_VERIFY_WITH_TIMEOUT(rejected.count() > 0, 5000);
}

void TestWKWebViewFrontend::routesSupportedNativeFileDrops()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString databasePath = directory.filePath(QStringLiteral("sample.sqlite"));
    QFile database(databasePath);
    QVERIFY(database.open(QIODevice::WriteOnly));
    database.close();

    const QString imagePath = directory.filePath(QStringLiteral("sample.png"));
    QImage image(2, 2, QImage::Format_ARGB32);
    QVERIFY(image.save(imagePath));

    WKWebViewAdapter adapter;
    QSignalSpy navigationRequested(adapter.nativeBridge(), &NativeBridge::navigationRequested);
    QSignalSpy imageDropAvailable(adapter.nativeBridge(), &NativeBridge::imageDropAvailable);

    QVERIFY(adapter.canAcceptDroppedFilePaths({databasePath}));
    adapter.routeDroppedFilePaths({databasePath});
    QCOMPARE(navigationRequested.count(), 1);
    QCOMPARE(navigationRequested.first().at(0).toString(), QStringLiteral("db"));

    QVERIFY(adapter.canAcceptDroppedFilePaths({imagePath}));
    adapter.routeDroppedFilePaths({imagePath});
    QCOMPARE(imageDropAvailable.count(), 1);
}

QTEST_MAIN(TestWKWebViewFrontend)

#include "test_wkwebview_frontend.moc"
