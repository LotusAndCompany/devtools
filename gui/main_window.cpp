#include "main_window.h"

#include "contents_area.h"
#include "gui/components/action_button.h"
#include "gui_application.h"
#include "menubar/about_devtools_dialog.h"
#include "menubar/settings_dialog.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QTranslator>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

static constexpr const char *kSidebarPaddingStyle = "padding: 8px 2px 8px 2px;";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_settingsDialog(new SettingsDialog(this))
{
    qDebug() << "=== MainWindow Constructor START ===";

    setObjectName("MainWindow");
    resize(1280, 720);
    setWindowTitle("DevTools");
    setAcceptDrops(true);

    setupCentralWidget();
    setupMenuBar();
    retranslateUi();

    connect(menuBar(), &QMenuBar::triggered, this, &MainWindow::onActionTriggered);
    connect(m_sidemenu, &Sidemenu::itemSelected, m_contentsArea,
            &ContentsArea::onSidemenuItemChanged);
    connect(m_sidemenuVisibilityButton, &QPushButton::clicked, this,
            &MainWindow::onSidemenuVisibilityButtonClicked);
    connect(m_windowAlwaysOnTopButton, &QPushButton::toggled, this,
            &MainWindow::onWindowAlwaysOnTopButtonToggled);
    connect(m_sidemenu, &Sidemenu::itemSelected, this, &MainWindow::onSidemenuItemSelected);

    // 設定ダイアログを作成し、シグナルを接続
    qDebug() << "Creating SettingsDialog...";

    qDebug() << "Connecting signals...";
    connect(m_settingsDialog, &SettingsDialog::languageChanged, this,
            &MainWindow::onLanguageChanged);
    qDebug() << "SettingsDialog signals connected successfully";

    // Always on topボタンの初期状態を設定から読み込み
    QSettings settings;
    bool const alwaysOnTop = settings.value("window/alwaysOnTop", false).toBool();
    m_windowAlwaysOnTopButton->setChecked(alwaysOnTop);

    // 初回起動時のデフォルト値を書き込む
    if (!settings.contains("general/showSidemenuOnStartup")) {
        settings.setValue("general/showSidemenuOnStartup", true);
    }
    if (!settings.contains("general/showLastToolOnStartup")) {
        settings.setValue("general/showLastToolOnStartup", false);
    }

    bool const showSidebarOnStartup =
        settings.value("general/showSidemenuOnStartup", true).toBool();
    setSidemenuHidden(!showSidebarOnStartup);
    bool const showLastToolOnStartup =
        settings.value("general/showLastToolOnStartup", false).toBool();
    if (showLastToolOnStartup) {
        const int lastToolValue = settings.value("general/lastUsedTool", -1).toInt();
        if (lastToolValue > Sidemenu::ID_MIN &&
            lastToolValue < static_cast<int>(Sidemenu::ID::MAX)) {
            m_sidemenu->selectItem(static_cast<Sidemenu::ID>(lastToolValue));
        }
    }

    qDebug() << "=== MainWindow Constructor COMPLETED ===";
}

MainWindow::~MainWindow()
{
    // ウィンドウサイズと位置を保存
    QSettings settings;
    if (settings.value("window/rememberSize", true).toBool()) {
        settings.setValue("window/width", width());
        settings.setValue("window/height", height());
    }
    if (settings.value("window/rememberPosition", true).toBool()) {
        settings.setValue("window/x", x());
        settings.setValue("window/y", y());
    }
}

void MainWindow::setupCentralWidget()
{
    auto *const centralwidget = new QWidget(this);
    centralwidget->setObjectName("centralwidget");
    centralwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *const horizontalLayout = new QHBoxLayout(centralwidget);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    m_sidemenu = new Sidemenu(centralwidget);
    m_sidemenu->setObjectName("sidemenu");
    const QSizePolicy sidemenuPolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_sidemenu->setSizePolicy(sidemenuPolicy);
    m_sidemenu->setMinimumWidth(240);
    horizontalLayout->addWidget(m_sidemenu);

    auto *const verticalLayout = new QVBoxLayout();
    verticalLayout->setContentsMargins(6, 0, 6, 6);

    auto *const toolbarLayout = new QHBoxLayout();
    toolbarLayout->setContentsMargins(0, 0, 0, 0);

    m_sidemenuVisibilityButton = new QPushButton(centralwidget);
    m_sidemenuVisibilityButton->setObjectName("sidemenuVisibilityButton");
    m_sidemenuVisibilityButton->setMinimumSize(24, 24);
    m_sidemenuVisibilityButton->setStyleSheet(kSidebarPaddingStyle);
    m_sidemenuVisibilityButton->setIcon(QIcon::fromTheme("left_panel_close"));
    m_sidemenuVisibilityButton->setIconSize(QSize(24, 24));
    m_sidemenuVisibilityButton->setFlat(true);
    toolbarLayout->addWidget(m_sidemenuVisibilityButton);

    toolbarLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    m_windowAlwaysOnTopButton = new QPushButton(centralwidget);
    m_windowAlwaysOnTopButton->setObjectName("windowAlwaysOnTopButton");
    m_windowAlwaysOnTopButton->setMinimumSize(24, 24);
    m_windowAlwaysOnTopButton->setStyleSheet(kSidebarPaddingStyle);
    m_windowAlwaysOnTopButton->setIcon(QIcon::fromTheme("flip_to_front"));
    m_windowAlwaysOnTopButton->setIconSize(QSize(24, 24));
    m_windowAlwaysOnTopButton->setCheckable(true);
    m_windowAlwaysOnTopButton->setFlat(true);
    toolbarLayout->addWidget(m_windowAlwaysOnTopButton);

    verticalLayout->addLayout(toolbarLayout);

    m_contentsArea = new ContentsArea(centralwidget);
    m_contentsArea->setObjectName("contentsArea");
    const QSizePolicy contentsPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_contentsArea->setSizePolicy(contentsPolicy);
    verticalLayout->addWidget(m_contentsArea);

    horizontalLayout->addLayout(verticalLayout);

    setCentralWidget(centralwidget);
}

void MainWindow::setupMenuBar()
{
    auto *const bar = menuBar();

    m_menuFile = bar->addMenu(QString());
    m_menuDevTools = bar->addMenu(QString());

    m_actionCloseWindow = new QAction(this);
    m_actionCloseWindow->setShortcut(QKeySequence("Ctrl+W"));
    m_menuFile->addAction(m_actionCloseWindow);

    m_actionShowMainWindow = new QAction(this);
    m_menuFile->addAction(m_actionShowMainWindow);

    m_actionAboutDevTools = new QAction(this);
    m_actionAboutDevTools->setMenuRole(QAction::AboutRole);
    m_menuDevTools->addAction(m_actionAboutDevTools);

    m_actionSettings = new QAction(this);
    m_actionSettings->setMenuRole(QAction::PreferencesRole);
    m_menuDevTools->addAction(m_actionSettings);
}

void MainWindow::retranslateUi()
{
    m_menuFile->setTitle(tr("File"));
    m_menuDevTools->setTitle("DevTools");
    m_actionCloseWindow->setText(tr("Close Window"));
    m_actionShowMainWindow->setText(tr("Show Main Window"));
    m_actionAboutDevTools->setText(tr("About DevTools"));
    m_actionSettings->setText(tr("Settings"));
}

#ifdef Q_OS_MACOS
void MainWindow::closeEvent(QCloseEvent *event)
{
    // ウィンドウサイズと位置を保存（macOSでは隠すだけなので、ここで保存）
    QSettings settings;
    if (settings.value("window/rememberSize", true).toBool()) {
        settings.setValue("window/width", width());
        settings.setValue("window/height", height());
    }
    if (settings.value("window/rememberPosition", true).toBool()) {
        settings.setValue("window/x", x());
        settings.setValue("window/y", y());
    }

    event->ignore();
    hide();
}
#endif

void MainWindow::onActionTriggered(QAction *action)
{
    if (action == m_actionAboutDevTools) {
        qDebug() << "About DevTools";
        QDialog *const aboutDialog = new AboutDevToolsDialog(this);
        aboutDialog->show();
    } else if (action == m_actionSettings) {
        qDebug() << "Settings";
        // 既存の設定ダイアログを表示（シグナルは既に接続済み）
        m_settingsDialog->show();
        m_settingsDialog->raise();
        m_settingsDialog->activateWindow();
    } else if (action == m_actionCloseWindow) {
        qDebug() << "Close Window";
        close();
    } else if (action == m_actionShowMainWindow) {
        qDebug() << "Show Main Window";
        show();
        raise();
        activateWindow();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::PaletteChange:
        emit colorSchemeChanged();
        event->accept();
        break;
    case QEvent::LanguageChange:
        // NOTE: メニューバーのAboutRoleとPreferencesRoleは翻訳できない
        //       ApplicationSpecificRoleにすれば翻訳は可能
        retranslateUi();
        event->accept();
        break;
    default:
        QMainWindow::changeEvent(event);
        break;
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (!droppedSQLiteFilePath(event->mimeData()).isEmpty()) {
        event->acceptProposedAction();
        return;
    }

    QMainWindow::dragEnterEvent(event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QString filePath = droppedSQLiteFilePath(event->mimeData());
    if (filePath.isEmpty()) {
        QMainWindow::dropEvent(event);
        return;
    }

    m_sidemenu->selectItem(Sidemenu::ID::DB_TOOL);
    m_contentsArea->openSQLiteFileInDbTool(filePath);
    event->acceptProposedAction();
}

void MainWindow::setSidemenuHidden(bool hide)
{
    if (hide) {
        m_sidemenuVisibilityButton->setIcon(QIcon::fromTheme("left_panel_open"));
        m_sidemenu->hide();
    } else {
        m_sidemenuVisibilityButton->setIcon(QIcon::fromTheme("left_panel_close"));
        m_sidemenu->show();
    }
}

bool MainWindow::isSQLiteFilePath(const QString &filePath)
{
    QFileInfo const fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    const QString suffix = fileInfo.suffix().toLower();
    return suffix == "db" || suffix == "sqlite" || suffix == "sqlite3";
}

QString MainWindow::droppedSQLiteFilePath(const QMimeData *mimeData)
{
    if (mimeData == nullptr || !mimeData->hasUrls()) {
        return {};
    }

    for (const QUrl &url : mimeData->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        const QString filePath = url.toLocalFile();
        if (isSQLiteFilePath(filePath)) {
            return filePath;
        }
    }

    return {};
}

void MainWindow::onSidemenuVisibilityButtonClicked()
{
    qDebug() << "MainWindow::onSidemenuVisibilityButtonClicked()";
    setSidemenuHidden(m_sidemenu->isVisible());
}

void MainWindow::onWindowAlwaysOnTopButtonToggled(bool checked)
{
    // TODO: Windowsでの挙動は要確認
    setWindowFlag(Qt::WindowStaysOnTopHint, checked);
    show();

    // 設定に保存
    QSettings settings;
    settings.setValue("window/alwaysOnTop", checked);
}

void MainWindow::onLanguageChanged(const QString &languageCode)
{
    qDebug() << "=== MainWindow::onLanguageChanged CALLED with:" << languageCode << "===";

    // GuiApplicationを取得してchangeLanguageメソッドを使用
    if (auto *app = qobject_cast<GuiApplication *>(QApplication::instance())) {
        if (app->changeLanguage(languageCode)) {
            // UIを再翻訳
            retranslateUi();
            qDebug() << ">>> Language change successful, UI retranslated";
        } else {
            qWarning() << ">>> Language change failed";
        }
    } else {
        qWarning() << ">>> Could not get GuiApplication instance!";
    }
    qDebug() << "=== MainWindow::onLanguageChanged COMPLETED ===";
}

void MainWindow::onSidemenuItemSelected(Sidemenu::ID id)
{
    QSettings settings;
    settings.setValue("general/lastUsedTool", static_cast<int>(id));
}
