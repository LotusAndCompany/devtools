#include "gui/gui_application.h"
#include <QApplicationStateChangeEvent>
#include <QDirIterator>
#include <QIcon>
#include <QStyleFactory>
#include <QStyleHints>

GuiApplication::GuiApplication(int argc, char **argv)
    : QApplication(argc, argv)
    , ApplicationMixin(argc, argv)
{
    connect(&window,
            &MainWindow::colorSchemeChanged,
            this,
            &GuiApplication::onWindowColorSchemeChanged);

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

    /*
    // 利用可能なstyleの確認
    for (const auto &key : QStyleFactory::keys()) {
        qDebug() << key;
    }
    */

    // リソースの確認
    /*
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const auto &name = it.next();
        if (!name.startsWith(":/qt-project.org"))
            qDebug() << name;
    }
    */

    // アイコンテーマの読み込み
    QStringList themeSearchPaths = QIcon::themeSearchPaths();
    themeSearchPaths.append(":/icons/dark");
    themeSearchPaths.append(":/icons/light");
    QIcon::setThemeSearchPaths(themeSearchPaths);

    applyColorScheme();
}

int GuiApplication::start()
{
    installEventFilter(this);

    // TODO: 前回のウィンドウサイズを記憶しておき、そのサイズで表示する (setWindowFlag(), setWindowFlags())
    window.show();
    return QApplication::exec();
}

void GuiApplication::applyColorScheme()
{
    switch (styleHints()->colorScheme()) {
    case Qt::ColorScheme::Unknown:
        qDebug() << "theme=Unknown";
        break;
    case Qt::ColorScheme::Light:
        qDebug() << "theme=Light";
        QIcon::setThemeName("light");
        break;
    case Qt::ColorScheme::Dark:
        qDebug() << "theme=Dark";
        QIcon::setThemeName("dark");
        break;
    }
}

void GuiApplication::onWindowColorSchemeChanged()
{
    applyColorScheme();
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
    qDebug() << "Application state changed";
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
