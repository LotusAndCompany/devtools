#include "gui_application.h"

#include <QApplicationStateChangeEvent>
#include <QDirIterator>
#include <QIcon>
#include <QSettings>
#include <QStyleFactory>
#include <QStyleHints>
#include <QTranslator>

GuiApplication::GuiApplication(int argc, char **argv)
    : QApplication(argc, argv), ApplicationMixin(AppType::GUI, argc, argv)
{
    QCoreApplication::setApplicationName("DevTools");

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
    // 設定ファイル等を読み込む
    QSettings settings;

    // ApplicationMixinのsetupApplicationが翻訳を初期化する
    setupApplication(this);

    // 保存された言語設定があれば、メインウィンドウ生成前に適用しておく
    const QString savedLanguage = settings.value("language", "").toString();
    if (!savedLanguage.isEmpty()) {
        const QString translationFile = QString(":i18n/dev-tools_%1.qm").arg(savedLanguage);
        if (mutableTranslator().load(translationFile)) {
            removeTranslator(&mutableTranslator());
            installTranslator(&mutableTranslator());
            qDebug() << "Applied saved language setting:" << savedLanguage;
        } else {
            qDebug() << "Failed to load saved language:" << translationFile;
        }
    }

    /*
    // 利用可能なstyleの確認
    for (const auto &key : QStyleFactory::keys()) {
        qDebug() << key;
    }
    */
    // setStyle(QStyleFactory::create("fusion"));

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
    themeSearchPaths.append(":/dark");
    themeSearchPaths.append(":/light");
    QIcon::setThemeSearchPaths(themeSearchPaths);

    // システムテーマを自動適用
    applyColorScheme();

    // 翻訳の適用が完了した後にメインウィンドウを生成
    window = std::make_unique<MainWindow>();

    connect(window.get(), &MainWindow::colorSchemeChanged, this,
            &GuiApplication::onWindowColorSchemeChanged);
}

bool GuiApplication::changeLanguage(const QString &languageCode)
{
    qDebug() << "GuiApplication::changeLanguage called with:" << languageCode;

    // 既存の翻訳を削除
    removeTranslator(&mutableTranslator());

    // 新しい翻訳を読み込み（正しいi18nパスを使用）
    QString translationFile = QString(":i18n/dev-tools_%1.qm").arg(languageCode);
    if (mutableTranslator().load(translationFile)) {
        installTranslator(&mutableTranslator());
        qDebug() << "Successfully changed language to:" << languageCode;
        return true;
    } else {
        qWarning() << "Failed to load translation:" << translationFile;
        // フォールバックを試行
        if (languageCode != "ja_JP" && mutableTranslator().load(":i18n/dev-tools_ja_JP.qm")) {
            installTranslator(&mutableTranslator());
            qDebug() << "Fallback to Japanese translation";
            return true;
        }
        return false;
    }
}

void GuiApplication::applySystemColorScheme()
{
    applyColorScheme();
}

int GuiApplication::start()
{
    QSettings settings;

    // ウィンドウサイズの復元
    if (settings.value("window/rememberSize", true).toBool()) {
        if (settings.contains("window/width") && settings.contains("window/height")) {
            int width = settings.value("window/width", 1280).toInt();
            int height = settings.value("window/height", 720).toInt();
            window->resize(width, height);
        }
    }

    // ウィンドウ位置の復元
    if (settings.value("window/rememberPosition", true).toBool()) {
        if (settings.contains("window/x") && settings.contains("window/y")) {
            int x = settings.value("window/x", 100).toInt();
            int y = settings.value("window/y", 100).toInt();
            window->move(x, y);
        }
    }

    // Always on top設定の適用
    if (settings.value("window/alwaysOnTop", false).toBool()) {
        window->setWindowFlag(Qt::WindowStaysOnTopHint, true);
    }

    window->show();

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
bool GuiApplication::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ApplicationStateChange:
        QApplication::event(event);
        if (applicationState() == Qt::ApplicationActive)
            window->show();
        return true;
    default:
        return QApplication::event(event);
    }
}

// NOTE:
// これでもDockからウィンドウを表示させる事はできるが、アプリケーション起動時にwindow.show()が2回呼ばれるかも?
/*
void GuiApplication::onApplicationStateChanged(Qt::ApplicationState state)
{
    qDebug() << "Application state changed";
    switch (state) {
    case Qt::ApplicationInactive:
        window->hide();
        break;
    case Qt::ApplicationActive:
        window->show();
        break;
    default:
    }
}
*/
#endif
