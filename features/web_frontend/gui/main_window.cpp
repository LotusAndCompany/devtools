#include "features/framework/gui/main_window.h"

#include "features/framework/core/application_settings_service.h"
#include "features/framework/core/local_file_selection_service.h"
#include "features/framework/gui/gui_application.h"
#include "features/image/core/image_processing_service.h"
#include "features/web_frontend/gui/native_bridge.h"
#ifdef Q_OS_MACOS
#include "features/web_frontend/gui/wk_webview_adapter.h"
#else
#include "features/web_frontend/gui/qt_webengine_adapter.h"
#endif

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setObjectName(QStringLiteral("MainWindow"));
    resize(1280, 720);
    setWindowTitle(QStringLiteral("DevTools"));
    setAcceptDrops(true);

    setupCentralWidget();
    setupMenuBar();
    retranslateUi();
    connect(menuBar(), &QMenuBar::triggered, this, &MainWindow::onActionTriggered);
    connect(native_bridge_, &NativeBridge::languageChangeRequested, this,
            [](const QString &languageCode) { MainWindow::onLanguageChanged(languageCode); });

    const ApplicationSettings settings = ApplicationSettingsService().read();
    setWindowFlag(Qt::WindowStaysOnTopHint, settings.alwaysOnTop);
}

MainWindow::~MainWindow()
{
    ApplicationSettingsService().saveWindowGeometry(width(), height(), x(), y());
}

void MainWindow::setupCentralWidget()
{
    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
#ifdef Q_OS_MACOS
    auto *host = new WKWebViewAdapter(central);
#else
    auto *host = new QtWebEngineAdapter(central);
#endif
    web_frontend_ = host;
    native_bridge_ = host->nativeBridge();
    layout->addWidget(web_frontend_);
    setCentralWidget(central);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *const bar = menuBar();
    menu_file_ = bar->addMenu(QString());
    menu_devtools_ = bar->addMenu(QString());

    action_close_ = new QAction(this);
    action_close_->setShortcut(QKeySequence(QStringLiteral("Ctrl+W")));
    menu_file_->addAction(action_close_);

    action_show_window_ = new QAction(this);
    menu_file_->addAction(action_show_window_);

    action_about_ = new QAction(this);
    action_about_->setMenuRole(QAction::AboutRole);
    menu_devtools_->addAction(action_about_);

    action_settings_ = new QAction(this);
    action_settings_->setMenuRole(QAction::PreferencesRole);
    menu_devtools_->addAction(action_settings_);
}

void MainWindow::retranslateUi()
{
    menu_file_->setTitle(tr("File"));
    menu_devtools_->setTitle(QStringLiteral("DevTools"));
    action_close_->setText(tr("Close Window"));
    action_show_window_->setText(tr("Show Main Window"));
    action_about_->setText(tr("About DevTools"));
    action_settings_->setText(tr("Settings"));
}

#ifdef Q_OS_MACOS
void MainWindow::closeEvent(QCloseEvent *event)
{
    ApplicationSettingsService().saveWindowGeometry(width(), height(), x(), y());
    event->ignore();
    hide();
}
#endif

void MainWindow::onActionTriggered(QAction *action)
{
    if (action == action_about_) {
        show();
        raise();
        activateWindow();
        native_bridge_->navigateTo(QStringLiteral("about"));
    } else if (action == action_settings_) {
        show();
        raise();
        activateWindow();
        native_bridge_->navigateTo(QStringLiteral("settings"));
    } else if (action == action_close_) {
        close();
    } else if (action == action_show_window_) {
        show();
        raise();
        activateWindow();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (!droppedSQLiteFilePath(event->mimeData()).isEmpty() ||
        !droppedImageFilePath(event->mimeData()).isEmpty()) {
        event->acceptProposedAction();
        return;
    }
    QMainWindow::dragEnterEvent(event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *const mimeData = event->mimeData();
    const QString databasePath = droppedSQLiteFilePath(mimeData);
    if (!databasePath.isEmpty()) {
        native_bridge_->openDroppedSQLiteFile(databasePath);
        event->acceptProposedAction();
        return;
    }
    const QString imagePath = droppedImageFilePath(mimeData);
    if (imagePath.isEmpty()) {
        QMainWindow::dropEvent(event);
        return;
    }
    native_bridge_->openDroppedImageFile(imagePath);
    event->acceptProposedAction();
}

QString MainWindow::droppedSQLiteFilePath(const QMimeData *mimeData)
{
    if (mimeData == nullptr || !mimeData->hasUrls()) {
        return {};
    }
    return LocalFileSelectionService::firstExistingLocalFile(
        mimeData->urls(),
        {QStringLiteral("db"), QStringLiteral("sqlite"), QStringLiteral("sqlite3")});
}

QString MainWindow::droppedImageFilePath(const QMimeData *mimeData)
{
    if (mimeData == nullptr || !mimeData->hasUrls()) {
        return {};
    }
    for (const QUrl &url : mimeData->urls()) {
        if (url.isLocalFile() && ImageProcessingService::isReadableImageFile(url.toLocalFile())) {
            return url.toLocalFile();
        }
    }
    return {};
}

void MainWindow::onLanguageChanged(const QString &languageCode)
{
    if (auto *app = qobject_cast<GuiApplication *>(QApplication::instance())) {
        if (!app->changeLanguage(languageCode)) {
            qWarning() << "Failed to change language to" << languageCode;
        }
    }
}
