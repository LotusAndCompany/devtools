#ifndef APPLICATION_SETTINGS_SERVICE_H
#define APPLICATION_SETTINGS_SERVICE_H

#include <QString>

#include <memory>

class QSettings;

struct ApplicationSettings
{
    QString language{"en"};
    bool hasSavedLanguage{false};
    bool showSidemenuOnStartup{true};
    bool showLastToolOnStartup{false};
    int lastUsedTool{-1};
    bool alwaysOnTop{false};
    bool rememberWindowSize{true};
    bool rememberWindowPosition{true};
    bool hasWindowSize{false};
    int windowWidth{0};
    int windowHeight{0};
    bool hasWindowPosition{false};
    int windowX{0};
    int windowY{0};
};

class ApplicationSettingsService
{
public:
    ApplicationSettingsService();
    explicit ApplicationSettingsService(QSettings &settings);
    ~ApplicationSettingsService();

    [[nodiscard]] ApplicationSettings read();
    void savePreferences(const ApplicationSettings &values);
    void saveWindowGeometry(int width, int height, int x, int y);
    void saveAlwaysOnTop(bool alwaysOnTop);
    void saveLastUsedTool(int toolId);

private:
    std::unique_ptr<QSettings> ownedSettings;
    QSettings *settings{nullptr};
};

#endif // APPLICATION_SETTINGS_SERVICE_H
