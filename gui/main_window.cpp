#include "main_window.h"
#include "./ui_main_window.h"
#include "menubar/about_devtools_dialog.h"

#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

#ifdef Q_OS_MACOS
void MainWindow::closeEvent(QCloseEvent *event)
{
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
}
