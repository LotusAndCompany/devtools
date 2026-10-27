#include "features/framework/core/application_settings_service.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

class TestApplicationSettingsService : public QObject
{
    Q_OBJECT

private slots:
    void readsDefaultsAndInitializesSidebarSettings();
    void savesPreferencesUsingExistingKeys();
    void savesWindowGeometryOnlyWhenEnabled();
};

void TestApplicationSettingsService::readsDefaultsAndInitializesSidebarSettings()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QSettings settings(tempDir.filePath(QStringLiteral("settings.ini")), QSettings::IniFormat);
    ApplicationSettingsService service(settings);

    const ApplicationSettings values = service.read();

    QCOMPARE(values.language, QStringLiteral("en"));
    QVERIFY(!values.hasSavedLanguage);
    QVERIFY(values.showSidemenuOnStartup);
    QVERIFY(!values.showLastToolOnStartup);
    QCOMPARE(values.lastUsedTool, -1);
    QVERIFY(!values.alwaysOnTop);
    QVERIFY(values.rememberWindowSize);
    QVERIFY(values.rememberWindowPosition);
    QVERIFY(!values.hasWindowSize);
    QVERIFY(!values.hasWindowPosition);
    QVERIFY(settings.contains(QStringLiteral("general/showSidemenuOnStartup")));
    QVERIFY(settings.contains(QStringLiteral("general/showLastToolOnStartup")));
    QVERIFY(!settings.contains(QStringLiteral("language")));
}

void TestApplicationSettingsService::savesPreferencesUsingExistingKeys()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QSettings settings(tempDir.filePath(QStringLiteral("settings.ini")), QSettings::IniFormat);
    settings.setValue(QStringLiteral("unrelated/value"), QStringLiteral("keep"));
    ApplicationSettingsService service(settings);
    ApplicationSettings values = service.read();
    values.language = QStringLiteral("ja_JP");
    values.showSidemenuOnStartup = false;
    values.showLastToolOnStartup = true;
    values.alwaysOnTop = true;
    values.rememberWindowSize = false;
    values.rememberWindowPosition = false;

    service.savePreferences(values);

    QCOMPARE(settings.value(QStringLiteral("language")).toString(), QStringLiteral("ja_JP"));
    QVERIFY(!settings.value(QStringLiteral("general/showSidemenuOnStartup")).toBool());
    QVERIFY(settings.value(QStringLiteral("general/showLastToolOnStartup")).toBool());
    QVERIFY(settings.value(QStringLiteral("window/alwaysOnTop")).toBool());
    QVERIFY(!settings.value(QStringLiteral("window/rememberSize")).toBool());
    QVERIFY(!settings.value(QStringLiteral("window/rememberPosition")).toBool());
    QCOMPARE(settings.value(QStringLiteral("unrelated/value")).toString(), QStringLiteral("keep"));

    QSettings persisted(tempDir.filePath(QStringLiteral("settings.ini")), QSettings::IniFormat);
    QCOMPARE(persisted.value(QStringLiteral("language")).toString(), QStringLiteral("ja_JP"));
}

void TestApplicationSettingsService::savesWindowGeometryOnlyWhenEnabled()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QSettings settings(tempDir.filePath(QStringLiteral("settings.ini")), QSettings::IniFormat);
    settings.setValue(QStringLiteral("window/rememberSize"), false);
    settings.setValue(QStringLiteral("window/rememberPosition"), true);
    settings.setValue(QStringLiteral("window/width"), 800);
    settings.setValue(QStringLiteral("window/height"), 600);
    ApplicationSettingsService service(settings);

    service.saveWindowGeometry(1200, 900, 25, 35);

    QCOMPARE(settings.value(QStringLiteral("window/width")).toInt(), 800);
    QCOMPARE(settings.value(QStringLiteral("window/height")).toInt(), 600);
    QCOMPARE(settings.value(QStringLiteral("window/x")).toInt(), 25);
    QCOMPARE(settings.value(QStringLiteral("window/y")).toInt(), 35);
    service.saveAlwaysOnTop(true);
    service.saveLastUsedTool(4);
    QVERIFY(settings.value(QStringLiteral("window/alwaysOnTop")).toBool());
    QCOMPARE(settings.value(QStringLiteral("general/lastUsedTool")).toInt(), 4);
}

QTEST_GUILESS_MAIN(TestApplicationSettingsService)
#include "test_application_settings_service.moc"
