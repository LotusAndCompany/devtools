#include "application_settings_service.h"

#include <QSettings>

ApplicationSettingsService::ApplicationSettingsService()
    : ownedSettings(std::make_unique<QSettings>()), settings(ownedSettings.get())
{}

ApplicationSettingsService::ApplicationSettingsService(QSettings &settings) : settings(&settings) {}

ApplicationSettingsService::~ApplicationSettingsService() = default;

ApplicationSettings ApplicationSettingsService::read()
{
    ApplicationSettings result;
    result.hasSavedLanguage = settings->contains(QStringLiteral("language"));
    result.language = settings->value(QStringLiteral("language"), QStringLiteral("en")).toString();

    const QString showSidebarKey = QStringLiteral("general/showSidemenuOnStartup");
    if (!settings->contains(showSidebarKey)) {
        settings->setValue(showSidebarKey, true);
    }
    result.showSidemenuOnStartup = settings->value(showSidebarKey, true).toBool();

    const QString showLastToolKey = QStringLiteral("general/showLastToolOnStartup");
    if (!settings->contains(showLastToolKey)) {
        settings->setValue(showLastToolKey, false);
    }
    result.showLastToolOnStartup = settings->value(showLastToolKey, false).toBool();
    result.lastUsedTool = settings->value(QStringLiteral("general/lastUsedTool"), -1).toInt();

    result.alwaysOnTop = settings->value(QStringLiteral("window/alwaysOnTop"), false).toBool();
    result.rememberWindowSize =
        settings->value(QStringLiteral("window/rememberSize"), true).toBool();
    result.rememberWindowPosition =
        settings->value(QStringLiteral("window/rememberPosition"), true).toBool();

    result.hasWindowSize = settings->contains(QStringLiteral("window/width")) &&
                           settings->contains(QStringLiteral("window/height"));
    result.windowWidth = settings->value(QStringLiteral("window/width"), 0).toInt();
    result.windowHeight = settings->value(QStringLiteral("window/height"), 0).toInt();
    result.hasWindowPosition = settings->contains(QStringLiteral("window/x")) &&
                               settings->contains(QStringLiteral("window/y"));
    result.windowX = settings->value(QStringLiteral("window/x"), 100).toInt();
    result.windowY = settings->value(QStringLiteral("window/y"), 100).toInt();
    return result;
}

void ApplicationSettingsService::savePreferences(const ApplicationSettings &values)
{
    settings->setValue(QStringLiteral("language"), values.language);
    settings->setValue(QStringLiteral("general/showSidemenuOnStartup"),
                       values.showSidemenuOnStartup);
    settings->setValue(QStringLiteral("general/showLastToolOnStartup"),
                       values.showLastToolOnStartup);
    settings->setValue(QStringLiteral("window/alwaysOnTop"), values.alwaysOnTop);
    settings->setValue(QStringLiteral("window/rememberSize"), values.rememberWindowSize);
    settings->setValue(QStringLiteral("window/rememberPosition"), values.rememberWindowPosition);
    settings->sync();
}

void ApplicationSettingsService::saveWindowGeometry(int width, int height, int x, int y)
{
    if (settings->value(QStringLiteral("window/rememberSize"), true).toBool()) {
        settings->setValue(QStringLiteral("window/width"), width);
        settings->setValue(QStringLiteral("window/height"), height);
    }
    if (settings->value(QStringLiteral("window/rememberPosition"), true).toBool()) {
        settings->setValue(QStringLiteral("window/x"), x);
        settings->setValue(QStringLiteral("window/y"), y);
    }
}

void ApplicationSettingsService::saveAlwaysOnTop(bool alwaysOnTop)
{
    settings->setValue(QStringLiteral("window/alwaysOnTop"), alwaysOnTop);
}

void ApplicationSettingsService::saveLastUsedTool(int toolId)
{
    settings->setValue(QStringLiteral("general/lastUsedTool"), toolId);
}
