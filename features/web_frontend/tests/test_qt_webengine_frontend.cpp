#include "features/web_frontend/gui/native_bridge.h"
#include "features/web_frontend/gui/qt_webengine_adapter.h"

#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QSignalSpy>
#include <QTemporaryDir>
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
    void routesDroppedFilesFromWebEngineContentToNativeBridge();
    void reannouncesDroppedImageAfterCancellingImageDialog();
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

    QTemporaryDir databaseDirectory;
    QVERIFY(databaseDirectory.isValid());
    const QString firstDatabasePath = databaseDirectory.filePath(QStringLiteral("first.sqlite"));
    QFile firstDatabase(firstDatabasePath);
    QVERIFY(firstDatabase.open(QIODevice::WriteOnly));
    firstDatabase.close();

    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral("document.querySelectorAll('.tool-nav .nav-item')[5].click(); true"),
        &result));
    view.nativeBridge()->openDroppedSQLiteFile(firstDatabasePath);
    const QString selectedFileScript =
        QStringLiteral("document.querySelector('.db-sidebar .selected-file')?.textContent ?? ''");
    QTRY_VERIFY_WITH_TIMEOUT(evaluateJavaScript(view.page(), selectedFileScript, &result) &&
                                 result.toString() == QFileInfo(firstDatabasePath).fileName(),
                             5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(view.page(),
                           QStringLiteral("Boolean(document.querySelector('.connected-card'))"),
                           &result) &&
            result.toBool(),
        10000);

    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral("(() => { const field = document.querySelector('.db-workspace textarea'); "
                       "if (!field) return ''; "
                       "Object.getOwnPropertyDescriptor(HTMLTextAreaElement.prototype, 'value')"
                       ".set.call(field, 'SELECT 123;'); "
                       "field.dispatchEvent(new Event('input', { bubbles: true })); "
                       "return field.value; })()"),
        &result));
    QCOMPARE(result.toString(), QStringLiteral("SELECT 123;"));

    const QString secondDatabasePath = databaseDirectory.filePath(QStringLiteral("second.sqlite"));
    QFile secondDatabase(secondDatabasePath);
    QVERIFY(secondDatabase.open(QIODevice::WriteOnly));
    secondDatabase.close();
    view.nativeBridge()->openDroppedSQLiteFile(secondDatabasePath);
    QTRY_VERIFY_WITH_TIMEOUT(evaluateJavaScript(view.page(), selectedFileScript, &result) &&
                                 result.toString() == QFileInfo(secondDatabasePath).fileName(),
                             5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(
            view.page(),
            QStringLiteral("document.querySelector('.db-workspace textarea')?.value ?? ''"),
            &result) &&
            result.toString() ==
                QStringLiteral(
                    "SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY name;"),
        5000);
}

void TestQtWebEngineFrontend::routesDroppedFilesFromWebEngineContentToNativeBridge()
{
    QtWebEngineAdapter view;
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString imagePath = directory.filePath(QStringLiteral("drop.png"));
    QImage image(2, 2, QImage::Format_ARGB32);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath, "PNG"));

    QSignalSpy imageAvailable(view.nativeBridge(), &NativeBridge::imageDropAvailable);
    QSignalSpy navigationRequested(view.nativeBridge(), &NativeBridge::navigationRequested);
    QVERIFY(imageAvailable.isValid());
    QVERIFY(navigationRequested.isValid());

    const auto sendFileDrop = [&view](const QUrl &url) {
        QMimeData mimeData;
        mimeData.setUrls({url});
        QDragEnterEvent dragEnter(QPoint(1, 1), Qt::CopyAction, &mimeData, Qt::LeftButton,
                                  Qt::NoModifier);
        QApplication::sendEvent(&view, &dragEnter);
        if (!dragEnter.isAccepted()) {
            return false;
        }
        QDropEvent drop(QPointF(1, 1), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&view, &drop);
        return drop.isAccepted();
    };

    QVERIFY(sendFileDrop(QUrl::fromLocalFile(imagePath)));
    QCOMPARE(imageAvailable.count(), 1);

    const QString databasePath = directory.filePath(QStringLiteral("drop.sqlite"));
    QFile database(databasePath);
    QVERIFY(database.open(QIODevice::WriteOnly));
    database.close();
    QVERIFY(sendFileDrop(QUrl::fromLocalFile(databasePath)));
    QCOMPARE(navigationRequested.count(), 1);
    QCOMPARE(navigationRequested.constFirst().constFirst().toString(), QStringLiteral("db"));
}

void TestQtWebEngineFrontend::reannouncesDroppedImageAfterCancellingImageDialog()
{
    QtWebEngineAdapter view;
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString imagePath = directory.filePath(QStringLiteral("pending.png"));
    QImage image(2, 2, QImage::Format_ARGB32);
    image.fill(Qt::blue);
    QVERIFY(image.save(imagePath, "PNG"));

    NativeBridge *const bridge = view.nativeBridge();
    QSignalSpy available(bridge, &NativeBridge::imageDropAvailable);
    QSignalSpy cancelled(bridge, &NativeBridge::operationCancelled);
    QVERIFY(available.isValid());
    QVERIFY(cancelled.isValid());

    bridge->startOperation({{QStringLiteral("contractVersion"), QStringLiteral("1")},
                            {QStringLiteral("requestId"), QStringLiteral("image-open-cancel")},
                            {QStringLiteral("operation"), QStringLiteral("imageOpen")},
                            {QStringLiteral("payload"), QVariantMap{}}});
    QFileDialog *const dialog = view.findChild<QFileDialog *>();
    QVERIFY(dialog != nullptr);

    bridge->openDroppedImageFile(imagePath);
    QCOMPARE(available.count(), 1);
    dialog->reject();

    QTRY_COMPARE_WITH_TIMEOUT(cancelled.count(), 1, 3000);
    QCOMPARE(available.count(), 2);
}

QTEST_MAIN(TestQtWebEngineFrontend)

#include "test_qt_webengine_frontend.moc"
