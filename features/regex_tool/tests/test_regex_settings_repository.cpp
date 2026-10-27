#include "features/regex_tool/core/regex_settings_repository.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

class TestRegexSettingsRepository : public QObject
{
    Q_OBJECT

private slots:
    void readsExistingDefaults();
    void savesExistingKeysAndReloads();
};

void TestRegexSettingsRepository::readsExistingDefaults()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QSettings settings(tempDir.filePath(QStringLiteral("settings.ini")), QSettings::IniFormat);
    devtools::RegexSettingsRepository repository(settings);

    const devtools::RegexSettings values = repository.read();

    QVERIFY(values.pattern.isEmpty());
    QVERIFY(values.testText.isEmpty());
    QVERIFY(values.replacePattern.isEmpty());
    QVERIFY(values.global);
    QVERIFY(!values.caseInsensitive);
    QVERIFY(!values.multiline);
    QVERIFY(!values.dotMatchesEverything);
    QVERIFY(!values.useUnicodeProperties);
    QVERIFY(!values.extendedPatternSyntax);
    QVERIFY(settings.allKeys().isEmpty());
}

void TestRegexSettingsRepository::savesExistingKeysAndReloads()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = tempDir.filePath(QStringLiteral("settings.ini"));
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue(QStringLiteral("unrelated/value"), QStringLiteral("preserved"));
    devtools::RegexSettingsRepository repository(settings);
    devtools::RegexSettings values;
    values.pattern = QStringLiteral("(?<word>foo)");
    values.testText = QStringLiteral("foo foo");
    values.replacePattern = QStringLiteral("$<word>bar");
    values.global = false;
    values.caseInsensitive = true;
    values.multiline = true;
    values.dotMatchesEverything = true;
    values.useUnicodeProperties = true;
    values.extendedPatternSyntax = true;

    repository.save(values);

    QSettings persisted(path, QSettings::IniFormat);
    const devtools::RegexSettings restored = devtools::RegexSettingsRepository(persisted).read();
    QCOMPARE(restored.pattern, values.pattern);
    QCOMPARE(restored.testText, values.testText);
    QCOMPARE(restored.replacePattern, values.replacePattern);
    QCOMPARE(restored.global, values.global);
    QCOMPARE(restored.caseInsensitive, values.caseInsensitive);
    QCOMPARE(restored.multiline, values.multiline);
    QCOMPARE(restored.dotMatchesEverything, values.dotMatchesEverything);
    QCOMPARE(restored.useUnicodeProperties, values.useUnicodeProperties);
    QCOMPARE(restored.extendedPatternSyntax, values.extendedPatternSyntax);
    QCOMPARE(persisted.value(QStringLiteral("unrelated/value")).toString(),
             QStringLiteral("preserved"));
}

QTEST_GUILESS_MAIN(TestRegexSettingsRepository)
#include "test_regex_settings_repository.moc"
