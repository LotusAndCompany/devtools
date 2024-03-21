#ifndef GUI_APPLICATION_H
#define GUI_APPLICATION_H

#include <QApplication>
#include <QtSystemDetection>
#include "application/application_mixin.h"
#include "mainwindow.h"

class GuiApplication : public QApplication, public ApplicationMixin
{
    Q_OBJECT

    MainWindow window;

public:
    GuiApplication(int argc, char **argv);
    GuiApplication() = delete;
    virtual ~GuiApplication() = default;

    void setup() override;
    int start() override;

// Platform specific
#ifdef Q_OS_MACOS
public:
    bool eventFilter(QObject *o, QEvent *e) override;
/*
private slots:
    void onApplicationStateChanged(Qt::ApplicationState state);
*/
#endif
};

#endif // GUI_APPLICATION_H
