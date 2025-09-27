#include "main_window.h"
#include "./ui_main_window.h"
#include "menubar/about_devtools_dialog.h"
#include "menubar/settings_dialog.h"
#include "gui_application.h"

#include <QCloseEvent>
#include <QTranslator>
#include <QApplication>
#include <QSettings>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settingsDialog(nullptr)
{
    qDebug() << "=== MainWindow Constructor START ===";
    ui->setupUi(this);

    connect(ui->menubar, &QMenuBar::triggered, this, &MainWindow::onActionTriggered);
    connect(ui->sidemenu,
            &Sidemenu::itemSelected,
            ui->contentsArea,
            &ContentsArea::onSidemenuItemChanged);
    connect(ui->sidemenuVisibilityButton,
            &QPushButton::clicked,
            this,
            &MainWindow::onSidemenuVisibilityButtonClicked);
    connect(ui->windowAlwaysOnTopButton,
            &QPushButton::toggled,
            this,
            &MainWindow::onWindowAlwaysOnTopButtonToggled);
    
    // 設定ダイアログを作成し、シグナルを接続
    qDebug() << "Creating SettingsDialog...";
    m_settingsDialog = new SettingsDialog(this);
    qDebug() << "Connecting signals...";
    connect(m_settingsDialog, &SettingsDialog::languageChanged,
            this, &MainWindow::onLanguageChanged);
    qDebug() << "SettingsDialog signals connected successfully";
    
    // Always on topボタンの初期状態を設定から読み込み
    QSettings settings;
    bool alwaysOnTop = settings.value("window/alwaysOnTop", false).toBool();
    ui->windowAlwaysOnTopButton->setChecked(alwaysOnTop);
    
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
    
    delete ui;
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
    if (action == ui->actionAboutDevTools) {
        qDebug() << "About DevTools";
        QDialog *const aboutDialog = new AboutDevToolsDialog(this);
        aboutDialog->show();
    } else if (action == ui->actionSettings) {
        qDebug() << "Settings";
        // 既存の設定ダイアログを表示（シグナルは既に接続済み）
        m_settingsDialog->show();
        m_settingsDialog->raise();
        m_settingsDialog->activateWindow();
    } else if (action == ui->actionCloseWindow) {
        qDebug() << "Close Window";
        close();
    } else if (action == ui->actionShowMainWindow) {
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
        ui->retranslateUi(this);
        event->accept();
        break;
    default:
        QMainWindow::changeEvent(event);
        break;
    }
}

void MainWindow::onSidemenuVisibilityButtonClicked()
{
    qDebug() << "MainWindow::onSidemenuVisibilityButtonClicked()";
    if (ui->sidemenu->isVisible()) {
        ui->sidemenuVisibilityButton->setIcon(QIcon::fromTheme("left_panel_open"));
        ui->sidemenu->hide();
    } else {
        ui->sidemenuVisibilityButton->setIcon(QIcon::fromTheme("left_panel_close"));
        ui->sidemenu->show();
    }
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
    if (GuiApplication *app = qobject_cast<GuiApplication*>(QApplication::instance())) {
        if (app->changeLanguage(languageCode)) {
            // UIを再翻訳
            ui->retranslateUi(this);
            qDebug() << ">>> Language change successful, UI retranslated";
        } else {
            qWarning() << ">>> Language change failed";
        }
    } else {
        qWarning() << ">>> Could not get GuiApplication instance!";
    }
    qDebug() << "=== MainWindow::onLanguageChanged COMPLETED ===";
}

