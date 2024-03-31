#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

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
    ~MainWindow();

private:
    void changeEvent(QEvent *event) override;
#ifdef Q_OS_MACOS
    // NOTE: MacOSの場合は×ボタンが押された時にウィンドウを隠す
    //       ウィンドウを表示する処理はGuiApplication::event(QEvent *event)で実装
    void closeEvent(QCloseEvent *event) override;
#endif

signals:
    void colorSchemeChanged();

private slots:
    void onActionTriggered(QAction *action);
    void onSidemenuVisibilityButtonClicked();
    void onWindowAlwaysOnTopButtonToggled(bool checked);
};
#endif // MAIN_WINDOW_H
