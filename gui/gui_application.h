#ifndef GUI_APPLICATION_H
#define GUI_APPLICATION_H

#include <QApplication>
#include <QtSystemDetection>
#include "core/application_mixin.h"
#include "main_window.h"

class GuiApplication : public QApplication, public ApplicationMixin
{
    Q_OBJECT

    MainWindow window;

public:
    GuiApplication(int argc, char **argv);
    GuiApplication() = delete;

    inline QString language() { return translator().language(); }

    void setup() override;
    int start() override;

private:
    // NOTE: staticにできる
    void applyColorScheme();

private slots:
    // NOTE: staticにできる
    void onWindowColorSchemeChanged();

// Platform specific
#ifdef Q_OS_MACOS
private:
    // NOTE: MacOSの場合はDockのアイコンがクリックされた時にウィンドウを表示する
    //       ウィンドウを隠す処理はMainWindow::closeEvent(QCloseEvent *event)で実装
    bool event(QEvent *) override;
/*
private slots:
    void onApplicationStateChanged(Qt::ApplicationState state);
*/
#endif
};

#endif // GUI_APPLICATION_H
