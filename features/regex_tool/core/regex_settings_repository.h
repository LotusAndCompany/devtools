#ifndef REGEX_SETTINGS_REPOSITORY_H
#define REGEX_SETTINGS_REPOSITORY_H

#include <QString>

#include <memory>

class QSettings;

namespace devtools {

struct RegexSettings
{
    QString pattern;
    QString testText;
    QString replacePattern;
    bool global{true};
    bool caseInsensitive{false};
    bool multiline{false};
    bool dotMatchesEverything{false};
    bool useUnicodeProperties{false};
    bool extendedPatternSyntax{false};
};

class RegexSettingsRepository
{
public:
    RegexSettingsRepository();
    explicit RegexSettingsRepository(QSettings &settings);
    ~RegexSettingsRepository();

    [[nodiscard]] RegexSettings read() const;
    void save(const RegexSettings &values);

private:
    std::unique_ptr<QSettings> ownedSettings;
    QSettings *settings{nullptr};
};

} // namespace devtools

#endif // REGEX_SETTINGS_REPOSITORY_H
