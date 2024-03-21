#include "gui/mainwindow.h"
#include "./ui_mainwindow.h"

#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
