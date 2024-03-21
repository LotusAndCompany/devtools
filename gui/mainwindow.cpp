#include "gui/mainwindow.h"
#include "./ui_mainwindow.h"

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
    } else if (action == ui->actionSettings) {
        qDebug() << "Settings" << Qt::endl;
    }
}
