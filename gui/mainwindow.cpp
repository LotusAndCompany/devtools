#include "gui/mainwindow.h"
#include "./ui_mainwindow.h"
#include "gui/menubar/about_devtools_dialog.h"

#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->menubar, &QMenuBar::triggered, this, &MainWindow::onActionTriggered);
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
        qDebug() << "About DevTools" << Qt::endl;
        QDialog *const aboutDialog = new AboutDevToolsDialog(this);
        aboutDialog->show();
    } else if (action == ui->actionSettings) {
        qDebug() << "Settings" << Qt::endl;
    }
}
