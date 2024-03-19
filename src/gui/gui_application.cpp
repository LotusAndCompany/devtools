#include "gui/gui_application.h"
#include <QApplicationStateChangeEvent>

GuiApplication::GuiApplication(int argc, char **argv)
    : QApplication(argc, argv)
    , ApplicationMixin(argc, argv)
{
#ifdef Q_OS_MACOS
    /*
    connect(this,
            &QGuiApplication::applicationStateChanged,
            this,
            &GuiApplication::onApplicationStateChanged);
    */
#endif
}

void GuiApplication::setup()
{
    // TODO: 設定ファイル等を読み込む

    setupApplication(this);
}

int GuiApplication::exec()
{
    installEventFilter(this);

    // TODO: 前回のウィンドウサイズを記憶しておき、そのサイズで表示する (setWindowFlag(), setWindowFlags())
    window.show();
    return QApplication::exec();
}

#ifdef Q_OS_MACOS
bool GuiApplication::eventFilter(QObject *o, QEvent *e)
{
    if (o == this) {
        switch (e->type()) {
        case QEvent::ApplicationStateChange:
            if (applicationState() == Qt::ApplicationActive)
                window.show();
            break;
        default:
            break;
        }
    }
    return QApplication::eventFilter(o, e);
}

// NOTE: これでもDockからウィンドウを表示させる事はできるが、アプリケーション起動時にwindow.show()が2回呼ばれるかも?
/*
void GuiApplication::onApplicationStateChanged(Qt::ApplicationState state)
{
    qDebug() << "Application state changed" << Qt::endl;
    switch (state) {
    case Qt::ApplicationInactive:
        window.hide();
        break;
    case Qt::ApplicationActive:
        window.show();
        break;
    default:
    }
}
*/
#endif
