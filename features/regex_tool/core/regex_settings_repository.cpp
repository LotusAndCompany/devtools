#include "regex_settings_repository.h"

#include <QSettings>

namespace devtools {

RegexSettingsRepository::RegexSettingsRepository()
    : ownedSettings(std::make_unique<QSettings>()), settings(ownedSettings.get())
{}

RegexSettingsRepository::RegexSettingsRepository(QSettings &settings) : settings(&settings) {}

RegexSettingsRepository::~RegexSettingsRepository() = default;

RegexSettings RegexSettingsRepository::read() const
{
    RegexSettings result;
    result.pattern = settings->value(QStringLiteral("RegexTester/pattern"), QString()).toString();
    result.testText = settings->value(QStringLiteral("RegexTester/testText"), QString()).toString();
    result.replacePattern =
        settings->value(QStringLiteral("RegexTester/replacePattern"), QString()).toString();
    result.global = settings->value(QStringLiteral("RegexTester/flagG"), true).toBool();
    result.caseInsensitive = settings->value(QStringLiteral("RegexTester/flagI"), false).toBool();
    result.multiline = settings->value(QStringLiteral("RegexTester/flagM"), false).toBool();
    result.dotMatchesEverything =
        settings->value(QStringLiteral("RegexTester/flagS"), false).toBool();
    result.useUnicodeProperties =
        settings->value(QStringLiteral("RegexTester/flagU"), false).toBool();
    result.extendedPatternSyntax =
        settings->value(QStringLiteral("RegexTester/flagX"), false).toBool();
    return result;
}

void RegexSettingsRepository::save(const RegexSettings &values)
{
    settings->setValue(QStringLiteral("RegexTester/pattern"), values.pattern);
    settings->setValue(QStringLiteral("RegexTester/testText"), values.testText);
    settings->setValue(QStringLiteral("RegexTester/replacePattern"), values.replacePattern);
    settings->setValue(QStringLiteral("RegexTester/flagG"), values.global);
    settings->setValue(QStringLiteral("RegexTester/flagI"), values.caseInsensitive);
    settings->setValue(QStringLiteral("RegexTester/flagM"), values.multiline);
    settings->setValue(QStringLiteral("RegexTester/flagS"), values.dotMatchesEverything);
    settings->setValue(QStringLiteral("RegexTester/flagU"), values.useUnicodeProperties);
    settings->setValue(QStringLiteral("RegexTester/flagX"), values.extendedPatternSyntax);
}

} // namespace devtools
