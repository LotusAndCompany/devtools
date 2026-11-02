#include "features/web_frontend/gui/native_bridge.h"
#include "features/web_frontend/gui/wk_webview_adapter.h"

#include <QEventLoop>
#include <QFile>
#include <QImage>
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
    void rendersEveryToolRoute();
    void blocksExternalMainFrameNavigation();
    void routesSupportedNativeFileDrops();

private:
    bool evaluateJavaScript(WKWebViewAdapter *adapter, const QString &script, QVariant *result);
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
