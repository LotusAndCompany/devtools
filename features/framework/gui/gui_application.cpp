#include "gui_application.h"

#include "design_system.h"
#include "features/framework/core/application_settings_service.h"
#include "icon_utils.h"

#include <QApplicationStateChangeEvent>
#include <QStyleHints>
#include <QTranslator>
#include <QWidget>

#include <oclero/qlementine.hpp>

GuiApplication::GuiApplication(int &argc, char **argv)
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
    const ApplicationSettings settings = ApplicationSettingsService().read();

    // ApplicationMixinのsetupApplicationが翻訳を初期化する
    setupApplication(this);

    // 保存された言語設定があれば、メインウィンドウ生成前に適用しておく
    if (settings.hasSavedLanguage && !settings.language.isEmpty()) {
        if (applyLanguage(settings.language)) {
            qDebug() << "Applied saved language setting:" << settings.language;
        }
    }

    // qlementineスタイルを適用
    qlementineStyle = new oclero::qlementine::QlementineStyle();
    setStyle(qlementineStyle);

    themeManager = new oclero::qlementine::ThemeManager(qlementineStyle, this);
    themeManager->loadDirectory(":/themes");

    connect(styleHints(), &QStyleHints::colorSchemeChanged, this,
            &GuiApplication::applyColorScheme);

    // アイコンフォントの読み込み
    material_symbols_loaded = IconUtils::initializeMaterialSymbols();

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

    if (applyLanguage(languageCode)) {
        qDebug() << "Successfully changed language to:" << languageCode;
        return true;
    }

    return false;
}

bool GuiApplication::applyLanguage(const QString &languageCode)
{
    // 既存の翻訳を削除
    removeTranslator(&mutableTranslator());

    if (languageCode == "en") {
        loadQtTranslator(this, languageCode);
        return true;
    }

    // 新しい翻訳を読み込み（正しいi18nパスを使用）
    QString const translationFile = QString(":i18n/dev-tools_%1.qm").arg(languageCode);
    if (mutableTranslator().load(translationFile)) {
        loadQtTranslator(this, languageCode);
        installTranslator(&mutableTranslator());
        return true;
    } else {
        qWarning() << "Failed to load translation:" << translationFile;
        return false;
    }
}

void GuiApplication::applySystemColorScheme()
{
    applyColorScheme();
}

int GuiApplication::start()
{
    const ApplicationSettings settings = ApplicationSettingsService().read();

    // ウィンドウサイズの復元
    if (settings.rememberWindowSize) {
        if (settings.hasWindowSize) {
            int const width = settings.windowWidth;
            int const height = settings.windowHeight;
            window->resize(width, height);
        }
    }

    // ウィンドウ位置の復元
    if (settings.rememberWindowPosition) {
        if (settings.hasWindowPosition) {
            window->move(settings.windowX, settings.windowY);
        }
    }

    // Always on top設定の適用
    if (settings.alwaysOnTop) {
        window->setWindowFlag(Qt::WindowStaysOnTopHint, true);
    }

    window->show();

    return QApplication::exec();
}

void GuiApplication::applyColorScheme()
{
    const auto scheme = styleHints()->colorScheme();
    const QString qlementineTheme = (scheme == Qt::ColorScheme::Dark) ? "Dark" : "Light";

    qDebug() << "theme=" << qlementineTheme;

    const bool themeChanged =
        themeManager != nullptr && themeManager->currentTheme() != qlementineTheme;
    if (themeChanged) {
        themeManager->setCurrentTheme(qlementineTheme);

        QApplication::setPalette(style()->standardPalette());
    }

    if (material_symbols_loaded) {
        IconUtils::refreshMaterialSymbolsTheme();
    }

    const auto allWidgets = QApplication::allWidgets();

    if (themeChanged) {
        for (auto *w : allWidgets) {
            w->setPalette(QPalette());
            w->update();
        }
        DevTools::Ui::refreshStatusColors();
        return;
    }

    for (auto *w : allWidgets) {
        if (!w->isVisible()) {
            continue;
        }
        w->update();
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
        if (applicationState() == Qt::ApplicationActive) {
            window->show();
        }
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
