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
    void loadsBundledReactAppAndRendersEveryRoute();
    void routesDroppedFilesFromWebEngineContentToNativeBridge();
    void reannouncesDroppedImageAfterCancellingImageDialog();
};

void TestQtWebEngineFrontend::loadsBundledReactAppAndRendersEveryRoute()
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
        "toolCount: document.querySelectorAll('.tool-nav .nav-item').length,"
        "main: Boolean(document.querySelector('main.app-main')),"
        "mounted: Boolean(document.querySelector('#root > *')),"
        "})");
    QJsonObject snapshot;
    bool rendered = false;
    for (int attempt = 0; attempt < 50; ++attempt) {
        QVariant result;
        QVERIFY(evaluateJavaScript(view.page(), snapshotScript, &result));
        snapshot = QJsonDocument::fromJson(result.toString().toUtf8()).object();
        if (snapshot.value(QStringLiteral("main")).toBool() &&
            snapshot.value(QStringLiteral("mounted")).toBool() &&
            snapshot.value(QStringLiteral("toolCount")).toInt() == 11) {
            rendered = true;
            break;
        }
        QTest::qWait(100);
    }

    QVERIFY2(rendered, "The bundled React app did not render its main shell and tool navigation.");
    QCOMPARE(snapshot.value(QStringLiteral("toolCount")).toInt(), 11);
    QVERIFY(snapshot.value(QStringLiteral("main")).toBool());
    QVERIFY(snapshot.value(QStringLiteral("mounted")).toBool());

    QVariant result;
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(
            view.page(), QStringLiteral("document.querySelector('.brand-logo')?.naturalWidth > 0"),
            &result) &&
            result.toBool(),
        5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(
            view.page(),
            QStringLiteral("document.fonts.check('20px \"Material Symbols Outlined\"', 'menu')"),
            &result) &&
            result.toBool(),
        5000);

    for (int index = 0; index < 11; ++index) {
        const QString clickScript =
            QStringLiteral("document.querySelectorAll('.tool-nav .nav-item')[%1].click(); true")
                .arg(index);
        QVERIFY(evaluateJavaScript(view.page(), clickScript, &result));

        QString headingText;
        for (int attempt = 0; attempt < 50; ++attempt) {
            QVariant heading;
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral("document.querySelector('.page-host:not([hidden]) .page-heading h1')"
                               "?.textContent ?? ''"),
                &heading));
            headingText = heading.toString().trimmed();
            if (!headingText.isEmpty()) {
                break;
            }
            QTest::qWait(100);
        }
        QVERIFY2(!headingText.isEmpty(),
                 qPrintable(QStringLiteral("Tool route %1 rendered no page heading.").arg(index)));
        QVERIFY(evaluateJavaScript(
            view.page(),
            QStringLiteral("document.querySelectorAll('.page-host:not([hidden]) .input-card, "
                           ".page-host:not([hidden]) .result-card').length > 0"),
            &result));
        QVERIFY2(result.toBool(),
                 qPrintable(QStringLiteral("Tool route %1 has no cards.").arg(index)));
        QVERIFY(evaluateJavaScript(
            view.page(),
            QStringLiteral(
                "Boolean(document.querySelector('.page-host:not([hidden]) > .empty-state'))"),
            &result));
        QVERIFY(!result.toBool());
        QVERIFY(evaluateJavaScript(
            view.page(),
            QStringLiteral(
                "Array.from(document.querySelectorAll('.page-host.active-page .button-row, "
                ".page-host.active-page .toolbar-row')).every((row) => { "
                "const buttons = Array.from(row.children).filter((child) => "
                "child.matches('button')); "
                "const first = buttons[0]?.getBoundingClientRect(); "
                "return buttons.length < 2 || buttons.every((button) => { "
                "const rect = button.getBoundingClientRect(); "
                "return Math.min(first.bottom, rect.bottom) - "
                "Math.max(first.top, rect.top) > 0; }); })"),
            &result));
        QVERIFY2(result.toBool(),
                 qPrintable(QStringLiteral("Button group wrapped on route %1").arg(index)));
        if (index == 1) {
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral("Math.round(parseFloat(getComputedStyle(document.querySelector("
                               "'.page-host.active-page .danger-button')).height)) === 28"),
                &result));
            QVERIFY(result.toBool());
        }
        if (index == 4) {
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "(() => { const cards = [...document.querySelectorAll("
                    "'.page-host.active-page .data-panel')]; "
                    "const first = cards[0]?.getBoundingClientRect(); "
                    "const second = cards[1]?.getBoundingClientRect(); "
                    "return cards.length === 2 && Math.abs(first.top - second.top) < 1 && "
                    "second.left > first.right; })()"),
                &result));
            QVERIFY2(result.toBool(), "Data conversion panels are not side by side.");
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral("(() => { const row = document.querySelector("
                               "'.page-host.active-page .data-output-toolbar'); "
                               "const controls = [...row.querySelectorAll(':scope > button'), "
                               "...row.querySelectorAll('.select-trigger')]; "
                               "const bottoms = controls.map((control) => "
                               "control.getBoundingClientRect().bottom); "
                               "return Math.max(...bottoms) - Math.min(...bottoms) < 1; })()"),
                &result));
            QVERIFY2(result.toBool(), "Data conversion toolbar controls are not bottom-aligned.");
        }
        if (index == 2) {
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral("parseFloat(getComputedStyle(document.querySelector("
                               "'.page-host:not([hidden]) .command-output')).marginTop) >= 12"),
                &result));
            QVERIFY(result.toBool());
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral("Math.round(parseFloat(getComputedStyle(document.querySelector("
                               "'.page-host:not([hidden]) [data-slot=button]')).height)) === 32"),
                &result));
            QVERIFY(result.toBool());
        }
    }

    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral("document.querySelectorAll('.tool-nav .nav-item')[6].click(); true"),
        &result));
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "document.querySelector('.page-host[data-page=qr] .select-trigger').click(); true"),
        &result));
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(
            view.page(),
            QStringLiteral(
                "Array.from(document.querySelectorAll('.select-popup .select-item')).some((item) "
                "=> item.textContent.trim() === 'Calendar')"),
            &result) &&
            result.toBool(),
        5000);
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "Array.from(document.querySelectorAll('.select-popup .select-item')).find((item) "
            "=> item.textContent.trim() === 'Calendar').click(); true"),
        &result));
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(view.page(),
                           QStringLiteral("Boolean(document.querySelector('.date-time-input'))"),
                           &result) &&
            result.toBool(),
        5000);
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "document.querySelector('.page-host[data-page=qr] .date-time-trigger').click(); true"),
        &result));
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(view.page(),
                           QStringLiteral("Boolean(document.querySelector('.date-time-popup'))"),
                           &result) &&
            result.toBool(),
        5000);
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral("(() => { const day = Array.from(document.querySelectorAll("
                       "'.date-time-popup .date-time-day')).find((button) => "
                       "!button.classList.contains('outside-month')); if (!day) return false; "
                       "day.click(); return true; })()"),
        &result));
    QVERIFY(result.toBool());
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(
            view.page(),
            QStringLiteral("/^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}$/.test(document.querySelector("
                           "'.page-host[data-page=qr] .date-time-input input').value) "
                           "&& !document.querySelector('input[type=datetime-local]')"),
            &result) &&
            result.toBool(),
        5000);

    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral("document.querySelectorAll('.tool-nav .nav-item')[1].click(); true"),
        &result));
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "(() => { const title = document.querySelector('.phrase-layout input'); "
            "const body = document.querySelector('.phrase-layout textarea'); "
            "Object.getOwnPropertyDescriptor(HTMLInputElement.prototype, 'value')"
            ".set.call(title, 'Unsaved draft'); "
            "title.dispatchEvent(new Event('input', { bubbles: true })); "
            "Object.getOwnPropertyDescriptor(HTMLTextAreaElement.prototype, 'value')"
            ".set.call(body, 'Draft content'); "
            "body.dispatchEvent(new Event('input', { bubbles: true })); "
            "document.querySelector('.template-list .toolbar-row button').click(); true; })()"),
        &result));
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(view.page(),
                           QStringLiteral("Boolean(document.querySelector('.confirm-dialog'))"),
                           &result) &&
            result.toBool(),
        5000);
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "document.querySelector('.confirm-dialog .dialog-footer button:first-of-type')"
            ".click(); true"),
        &result));
    QVERIFY(evaluateJavaScript(
        view.page(), QStringLiteral("document.querySelector('.phrase-layout input')?.value ?? ''"),
        &result));
    QCOMPARE(result.toString(), QStringLiteral("Unsaved draft"));

    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "document.querySelector('.template-list .toolbar-row button').click(); true"),
        &result));
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(view.page(),
                           QStringLiteral("Boolean(document.querySelector('.confirm-dialog "
                                          ".dialog-footer button:nth-of-type(2)'))"),
                           &result) &&
            result.toBool(),
        5000);
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "document.querySelector('.confirm-dialog .dialog-footer button:nth-of-type(2)')"
            ".click(); true"),
        &result));
    QVERIFY(evaluateJavaScript(
        view.page(), QStringLiteral("document.querySelector('.phrase-layout input')?.value ?? ''"),
        &result));
    QVERIFY(result.toString().isEmpty());

    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral("document.querySelectorAll('.tool-nav .nav-item')[4].click(); true"),
        &result));
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "(() => { const field = document.querySelector('.page-host:not([hidden]) textarea'); "
            "Object.getOwnPropertyDescriptor(HTMLTextAreaElement.prototype, 'value')"
            ".set.call(field, '{'); "
            "field.dispatchEvent(new Event('input', { bubbles: true })); "
            "document.querySelector('.page-host:not([hidden]) .data-output-toolbar > button')"
            ".click(); true; })()"),
        &result));
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral("document.querySelector('.page-host:not([hidden]) textarea')?.value ?? ''"),
        &result));
    QCOMPARE(result.toString(), QStringLiteral("{"));
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(
            view.page(),
            QStringLiteral("Boolean(document.querySelector('.message-dialog[role=alertdialog]'))"),
            &result) &&
            result.toBool(),
        5000);
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "document.querySelector('.message-dialog .dialog-title')?.textContent ?? ''"),
        &result));
    QVERIFY(result.toString() == QStringLiteral("Error") ||
            result.toString() == QStringLiteral("エラー"));
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "document.querySelector('.message-dialog .dialog-footer button').click(); true"),
        &result));

    for (const QString &selector : {QStringLiteral(".sidebar-bottom .nav-item:nth-child(1)"),
                                    QStringLiteral(".sidebar-bottom .nav-item:nth-child(2)"),
                                    QStringLiteral("aside .nav-item")}) {
        QVERIFY(evaluateJavaScript(
            view.page(), QStringLiteral("document.querySelector('%1').click(); true").arg(selector),
            &result));
        if (selector == QStringLiteral(".sidebar-bottom .nav-item:nth-child(2)")) {
            bool aboutDialogOpened = false;
            for (int attempt = 0; attempt < 50; ++attempt) {
                QVERIFY(evaluateJavaScript(
                    view.page(),
                    QStringLiteral("Boolean(document.querySelector('.about-dialog[role=dialog]'))"),
                    &result));
                aboutDialogOpened = result.toBool();
                if (aboutDialogOpened)
                    break;
                QTest::qWait(100);
            }
            QVERIFY(aboutDialogOpened);
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "document.querySelectorAll('.about-dialog [role=tab]')[1]?.click(); true"),
                &result));
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral("Boolean(document.querySelector('.about-dialog textarea')?.value)"),
                &result));
            QVERIFY(result.toBool());
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "document.querySelector('.about-dialog .dialog-footer button').click(); true"),
                &result));
            continue;
        }
        if (selector == QStringLiteral(".sidebar-bottom .nav-item:nth-child(1)")) {
            QTRY_VERIFY_WITH_TIMEOUT(
                evaluateJavaScript(
                    view.page(),
                    QStringLiteral(
                        "Boolean(document.querySelector('.settings-dialog[role=dialog]'))"),
                    &result) &&
                    result.toBool(),
                5000);
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "document.querySelector('.page-host.active-page')?.dataset.page ?? ''"),
                &result));
            QCOMPARE(result.toString(), QStringLiteral("data"));
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "document.querySelector('.settings-dialog .select-chevron')?.textContent"
                    ".trim() === 'keyboard_arrow_down'"),
                &result));
            QVERIFY(result.toBool());
            QTRY_VERIFY_WITH_TIMEOUT(
                evaluateJavaScript(view.page(),
                                   QStringLiteral("Boolean(document.querySelector('.settings-"
                                                  "dialog .select-trigger:not(:disabled)'))"),
                                   &result) &&
                    result.toBool(),
                5000);
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "document.querySelector('.settings-dialog .select-trigger').click(); true"),
                &result));
            QTRY_VERIFY_WITH_TIMEOUT(
                evaluateJavaScript(
                    view.page(),
                    QStringLiteral("Boolean(document.querySelector('.select-popup .select-item'))"),
                    &result) &&
                    result.toBool(),
                5000);
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "(() => { const item = document.querySelector('.select-popup .select-item'); "
                    "if (!item) return false; const rect = item.getBoundingClientRect(); "
                    "return document.elementFromPoint(rect.x + rect.width / 2, "
                    "rect.y + rect.height / 2)?.closest('.select-item') === item; })()"),
                &result));
            QVERIFY(result.toBool());
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "document.querySelector('.select-popup .select-item').click(); true"),
                &result));
            QTRY_VERIFY_WITH_TIMEOUT(
                evaluateJavaScript(
                    view.page(),
                    QStringLiteral(
                        "document.querySelector('.settings-dialog .select-trigger')?.textContent"
                        ".includes('English') ?? false"),
                    &result) &&
                    result.toBool(),
                5000);
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "document.querySelector('.settings-dialog .dialog-footer button:first-of-type')"
                    ".click(); true"),
                &result));
            QTRY_VERIFY_WITH_TIMEOUT(
                evaluateJavaScript(
                    view.page(),
                    QStringLiteral("!document.querySelector('.settings-dialog[role=dialog]')"),
                    &result) &&
                    result.toBool(),
                5000);
            continue;
        }
        bool pageRendered = false;
        for (int attempt = 0; attempt < 50; ++attempt) {
            QVERIFY(evaluateJavaScript(
                view.page(),
                QStringLiteral(
                    "Boolean(document.querySelector('.page-host:not([hidden]) .page-heading h1, "
                    ".page-host:not([hidden]) .tool-card-grid'))"),
                &result));
            pageRendered = result.toBool();
            if (pageRendered) {
                break;
            }
            QTest::qWait(100);
        }
        QVERIFY(pageRendered);
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
    QVERIFY(evaluateJavaScript(
        view.page(),
        QStringLiteral(
            "document.querySelector('.database-toolbar .connection-name')?.textContent ?? ''"),
        &result));
    QVERIFY(result.toString().isEmpty());
    view.nativeBridge()->openDroppedSQLiteFile(firstDatabasePath);
    const QString selectedFileScript = QStringLiteral(
        "document.querySelector('.database-toolbar .connection-name')?.textContent ?? ''");
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(view.page(), selectedFileScript, &result) &&
            result.toString() ==
                QStringLiteral("SQLite: %1").arg(QFileInfo(firstDatabasePath).fileName()),
        5000);

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
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(view.page(), selectedFileScript, &result) &&
            result.toString() ==
                QStringLiteral("SQLite: %1").arg(QFileInfo(secondDatabasePath).fileName()),
        5000);
    QTRY_VERIFY_WITH_TIMEOUT(
        evaluateJavaScript(
            view.page(),
            QStringLiteral("document.querySelector('.db-workspace textarea')?.value ?? ''"),
            &result) &&
            result.toString() == QStringLiteral("SELECT 123;"),
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

    bridge->startOperation({{QStringLiteral("contractVersion"), QStringLiteral("2")},
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
