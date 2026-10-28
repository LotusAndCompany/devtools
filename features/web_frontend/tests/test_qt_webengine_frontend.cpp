#include "features/web_frontend/gui/qt_webengine_adapter.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTimer>
#include <QWebEnginePage>
#include <QtTest>

#include <memory>

namespace {
struct JavaScriptEvaluation
{
    QEventLoop loop;
    QVariant result;
    bool finished = false;
};

bool evaluateJavaScript(QWebEnginePage *page, const QString &script, QVariant *result)
{
    const auto evaluation = std::make_shared<JavaScriptEvaluation>();
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &evaluation->loop, &QEventLoop::quit);
    page->runJavaScript(script, [evaluation](const QVariant &value) {
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
} // namespace

class TestQtWebEngineFrontend : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void loadsBundledReactAppConnectsNativeBridgeAndRendersEveryRoute();
};

void TestQtWebEngineFrontend::loadsBundledReactAppConnectsNativeBridgeAndRendersEveryRoute()
{
    QtWebEngineAdapter view;
    view.resize(1100, 760);
    QSignalSpy loaded(&view, &QWebEngineView::loadFinished);
    QVERIFY(loaded.isValid());
    view.show();

    QTRY_VERIFY_WITH_TIMEOUT(loaded.count() > 0, 20000);
    QVERIFY(loaded.constLast().constFirst().toBool());
    QCOMPARE(view.url(), QUrl(QStringLiteral("qrc:/frontend/index.html")));

    const QString snapshotScript = QStringLiteral(
        "JSON.stringify({"
        "status: document.querySelector('.topbar-status')?.innerText ?? '',"
        "contract: document.querySelector('.contract-label')?.innerText ?? '',"
        "toolCount: document.querySelectorAll('.tool-nav .nav-item').length,"
        "main: Boolean(document.querySelector('main.app-main'))"
        "})");
    QJsonObject snapshot;
    bool connected = false;
    for (int attempt = 0; attempt < 50; ++attempt) {
        QVariant result;
        QVERIFY(evaluateJavaScript(view.page(), snapshotScript, &result));
        snapshot = QJsonDocument::fromJson(result.toString().toUtf8()).object();
        const QString status = snapshot.value(QStringLiteral("status")).toString();
        if (status.contains(QStringLiteral("Connected")) ||
            status.contains(QStringLiteral("接続済み"))) {
            connected = true;
            break;
        }
        QTest::qWait(100);
    }

    QVERIFY2(connected, "The React page did not connect to NativeBridge through QWebChannel.");
    QCOMPARE(snapshot.value(QStringLiteral("contract")).toString(), QStringLiteral("Bridge 1"));
    QCOMPARE(snapshot.value(QStringLiteral("toolCount")).toInt(), 11);
    QVERIFY(snapshot.value(QStringLiteral("main")).toBool());

    QVariant result;
    for (int index = 0; index < 11; ++index) {
        const QString clickScript =
            QStringLiteral("document.querySelectorAll('.tool-nav .nav-item')[%1].click(); true")
                .arg(index);
        QVERIFY(evaluateJavaScript(view.page(), clickScript, &result));

        QVariant heading;
        QVERIFY(evaluateJavaScript(
            view.page(),
            QStringLiteral("document.querySelector('.page-host:not([hidden]) .page-heading h1')"
                           "?.textContent ?? ''"),
            &heading));
        QVERIFY2(!heading.toString().trimmed().isEmpty(),
                 qPrintable(QStringLiteral("Tool route %1 rendered no page heading.").arg(index)));
        QVERIFY(evaluateJavaScript(
            view.page(),
            QStringLiteral(
                "Boolean(document.querySelector('.page-host:not([hidden]) > .empty-state'))"),
            &result));
        QVERIFY(!result.toBool());
    }

    for (const QString &selector : {QStringLiteral(".sidebar-bottom .nav-item:nth-child(1)"),
                                    QStringLiteral(".sidebar-bottom .nav-item:nth-child(2)"),
                                    QStringLiteral("aside .nav-item")}) {
        QVERIFY(evaluateJavaScript(
            view.page(), QStringLiteral("document.querySelector('%1').click(); true").arg(selector),
            &result));
        QVERIFY(evaluateJavaScript(
            view.page(),
            QStringLiteral(
                "document.querySelector('.page-host:not([hidden]) h1')?.textContent ?? ''"),
            &result));
        QVERIFY(!result.toString().trimmed().isEmpty());
    }
}

QTEST_MAIN(TestQtWebEngineFrontend)

#include "test_qt_webengine_frontend.moc"
