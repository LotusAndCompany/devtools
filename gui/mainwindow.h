#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSystemDetection>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow *const ui;

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

#ifdef Q_OS_MACOS
    // MacOSの場合は×ボタンの動作を変える
    void closeEvent(QCloseEvent *event) override;
#endif

private slots:
    void onActionTriggered(QAction *action);
};
#endif // MAINWINDOW_H
