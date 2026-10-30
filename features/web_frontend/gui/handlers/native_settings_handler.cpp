#include "features/framework/core/application_settings_service.h"
#include "features/regex_tool/core/regex_settings_repository.h"
#include "features/web_frontend/gui/native_bridge.h"

// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "app_info.autogen.cpp"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QJsonObject>
#include <QSettings>
#include <QWidget>

namespace {
constexpr qint64 MAX_CLIPBOARD_IMAGE_PIXELS = 100'000'000;

bool isSupportedClipboardImage(const QImage &image)
{
    return !image.isNull() && image.width() > 0 && image.height() > 0 &&
           static_cast<qint64>(image.width()) * image.height() <= MAX_CLIPBOARD_IMAGE_PIXELS;
}
} // namespace

void NativeBridge::startRegexSettings(const QString &requestId, const QVariantMap &payload)
{
    devtools::RegexSettingsRepository repository;
    const QString action = payload.value(QStringLiteral("action")).toString();
    if (action == QStringLiteral("load")) {
        const devtools::RegexSettings settings = repository.read();
        completeOnUiThread(
            requestId,
            {{QStringLiteral("pattern"), settings.pattern},
             {QStringLiteral("testText"), settings.testText},
             {QStringLiteral("replacePattern"), settings.replacePattern},
             {QStringLiteral("options"),
              QVariantMap{
                  {QStringLiteral("global"), settings.global},
                  {QStringLiteral("caseInsensitive"), settings.caseInsensitive},
                  {QStringLiteral("multiline"), settings.multiline},
                  {QStringLiteral("dotMatchesEverything"), settings.dotMatchesEverything},
                  {QStringLiteral("useUnicodeProperties"), settings.useUnicodeProperties},
                  {QStringLiteral("extendedPatternSyntax"), settings.extendedPatternSyntax}}}});
        return;
    }

    if (action != QStringLiteral("save") ||
        payload.value(QStringLiteral("pattern")).metaType().id() != QMetaType::QString ||
        payload.value(QStringLiteral("testText")).metaType().id() != QMetaType::QString ||
        payload.value(QStringLiteral("replacePattern")).metaType().id() != QMetaType::QString ||
        payload.value(QStringLiteral("options")).metaType().id() != QMetaType::QVariantMap) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("Regular expression settings are malformed."));
        operations_.remove(requestId);
        return;
    }

    const QVariantMap options = payload.value(QStringLiteral("options")).toMap();
    static const QStringList booleanOptions = {QStringLiteral("global"),
                                               QStringLiteral("caseInsensitive"),
                                               QStringLiteral("multiline"),
                                               QStringLiteral("dotMatchesEverything"),
                                               QStringLiteral("useUnicodeProperties"),
                                               QStringLiteral("extendedPatternSyntax")};
    for (const QString &option : booleanOptions) {
        if (options.value(option).metaType().id() != QMetaType::Bool) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("Regular expression options must be boolean values."));
            operations_.remove(requestId);
            return;
        }
    }

    devtools::RegexSettings settings;
    settings.pattern = payload.value(QStringLiteral("pattern")).toString();
    settings.testText = payload.value(QStringLiteral("testText")).toString();
    settings.replacePattern = payload.value(QStringLiteral("replacePattern")).toString();
    settings.global = options.value(QStringLiteral("global")).toBool();
    settings.caseInsensitive = options.value(QStringLiteral("caseInsensitive")).toBool();
    settings.multiline = options.value(QStringLiteral("multiline")).toBool();
    settings.dotMatchesEverything = options.value(QStringLiteral("dotMatchesEverything")).toBool();
    settings.useUnicodeProperties = options.value(QStringLiteral("useUnicodeProperties")).toBool();
    settings.extendedPatternSyntax =
        options.value(QStringLiteral("extendedPatternSyntax")).toBool();
    if (settings.pattern.toUtf8().size() > MAX_INPUT_BYTES ||
        settings.testText.toUtf8().size() > MAX_INPUT_BYTES ||
        settings.replacePattern.toUtf8().size() > MAX_INPUT_BYTES) {
        fail(requestId, QStringLiteral("SIZE_LIMIT_EXCEEDED"),
             QStringLiteral("Regular expression settings are limited to 1 MiB each."));
        operations_.remove(requestId);
        return;
    }

    repository.save(settings);
    completeOnUiThread(requestId, {{QStringLiteral("saved"), true}});
}

void NativeBridge::startSettingsRead(const QString &requestId)
{
    const ApplicationSettings settings = ApplicationSettingsService().read();
    completeOnUiThread(
        requestId,
        {{QStringLiteral("language"), settings.language},
         {QStringLiteral("showSidebarOnStartup"), settings.showSidemenuOnStartup},
         {QStringLiteral("showLastToolOnStartup"), settings.showLastToolOnStartup},
         {QStringLiteral("lastUsedTool"), settings.lastUsedTool},
         {QStringLiteral("theme"),
          QSettings().value(QStringLiteral("general/theme"), QStringLiteral("system")).toString()},
         {QStringLiteral("alwaysOnTop"), settings.alwaysOnTop},
         {QStringLiteral("rememberWindowSize"), settings.rememberWindowSize},
         {QStringLiteral("rememberWindowPosition"), settings.rememberWindowPosition}});
}

void NativeBridge::startAboutInfo(const QString &requestId)
{
    const QString buildEnvironment = QString::fromUtf8(DevTools::OS_NAME) + QLatin1Char(' ') +
                                     QString::fromUtf8(DevTools::OS_VERSION) +
                                     QStringLiteral(", ") +
                                     QString::fromUtf8(DevTools::COMPILER_NAME) + QLatin1Char(' ') +
                                     QString::fromUtf8(DevTools::COMPILER_VERSION);
    QFile licenseFile(QStringLiteral(":/docs/LICENSE"));
    QString licenseText;
    if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        licenseText = QString::fromUtf8(licenseFile.readAll());
    }
    completeOnUiThread(
        requestId,
        {{QStringLiteral("appVersion"), QString::fromUtf8(DevTools::APP_VERSION)},
         {QStringLiteral("revision"), QString::fromUtf8(DevTools::GIT_REVISION)},
         {QStringLiteral("buildEnvironment"), buildEnvironment},
         {QStringLiteral("qtVersion"), QString::fromUtf8(qVersion())},
         {QStringLiteral("copyright"), tr("2025 © Lotus&Company Inc. All rights reserved.")},
         {QStringLiteral("licenseText"), licenseText}});
}

void NativeBridge::startSaveLastUsedTool(const QString &requestId, const QVariantMap &payload)
{
    bool valid = false;
    const int toolId = payload.value(QStringLiteral("toolId")).toInt(&valid);
    if (!valid || toolId < 1 || toolId > 13) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("The tool identifier is not supported."));
        operations_.remove(requestId);
        return;
    }
    ApplicationSettingsService().saveLastUsedTool(toolId);
    completeOnUiThread(requestId, {{QStringLiteral("saved"), true}});
}

void NativeBridge::startClipboard(const QString &requestId, const QVariantMap &payload)
{
    if (payload.contains(QStringLiteral("imageDataUrl"))) {
        const QVariant imageValue = payload.value(QStringLiteral("imageDataUrl"));
        const QString imageDataUrl = imageValue.toString();
        const QString prefix = QStringLiteral("data:image/png;base64,");
        if (imageValue.metaType().id() != QMetaType::QString || !imageDataUrl.startsWith(prefix) ||
            imageDataUrl.toUtf8().size() > MAX_MESSAGE_BYTES) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("The image clipboard data is invalid."));
            operations_.remove(requestId);
            return;
        }
        const QByteArray imageBytes = QByteArray::fromBase64(
            imageDataUrl.mid(prefix.size()).toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
        QBuffer buffer;
        buffer.setData(imageBytes);
        if (!buffer.open(QIODevice::ReadOnly)) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("The image clipboard data is invalid."));
            operations_.remove(requestId);
            return;
        }
        QImageReader reader(&buffer, "PNG");
        const QSize dimensions = reader.size();
        if (!dimensions.isValid() || static_cast<qint64>(dimensions.width()) * dimensions.height() >
                                         MAX_CLIPBOARD_IMAGE_PIXELS) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("The image clipboard data is invalid."));
            operations_.remove(requestId);
            return;
        }
        const QImage image = reader.read();
        if (!isSupportedClipboardImage(image)) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("The image clipboard data is invalid."));
            operations_.remove(requestId);
            return;
        }
        QApplication::clipboard()->setImage(image);
        completeOnUiThread(requestId, {{QStringLiteral("copied"), true}});
        return;
    }

    QString text;
    if (!textPayload(requestId, payload, QStringLiteral("Clipboard text is larger than 1 MiB."),
                     &text)) {
        operations_.remove(requestId);
        return;
    }
    QApplication::clipboard()->setText(text);
    completeOnUiThread(requestId, {{QStringLiteral("copied"), true}});
}
void NativeBridge::startSaveSettings(const QString &requestId, const QVariantMap &payload)
{
    static const QStringList boolFields = {
        QStringLiteral("showSidebarOnStartup"), QStringLiteral("showLastToolOnStartup"),
        QStringLiteral("alwaysOnTop"), QStringLiteral("rememberWindowSize"),
        QStringLiteral("rememberWindowPosition")};
    for (const QString &field : boolFields) {
        if (payload.value(field).metaType().id() != QMetaType::Bool) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("All preference fields must be boolean values."));
            operations_.remove(requestId);
            return;
        }
    }
    const QString language = payload.value(QStringLiteral("language")).toString();
    if (language != QStringLiteral("en") && language != QStringLiteral("ja_JP")) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("Language must be en or ja_JP."));
        operations_.remove(requestId);
        return;
    }
    const QString theme = payload.value(QStringLiteral("theme")).toString();
    if (theme != QStringLiteral("system") && theme != QStringLiteral("light") &&
        theme != QStringLiteral("dark")) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("Theme must be system, light or dark."));
        operations_.remove(requestId);
        return;
    }

    ApplicationSettingsService service;
    ApplicationSettings settings = service.read();
    const QString previousLanguage = settings.language;
    settings.language = language;
    settings.showSidemenuOnStartup = payload.value(QStringLiteral("showSidebarOnStartup")).toBool();
    settings.showLastToolOnStartup =
        payload.value(QStringLiteral("showLastToolOnStartup")).toBool();
    settings.alwaysOnTop = payload.value(QStringLiteral("alwaysOnTop")).toBool();
    settings.rememberWindowSize = payload.value(QStringLiteral("rememberWindowSize")).toBool();
    settings.rememberWindowPosition =
        payload.value(QStringLiteral("rememberWindowPosition")).toBool();
    service.savePreferences(settings);
    QSettings().setValue(QStringLiteral("general/theme"), theme);

    if (dialog_parent_ != nullptr) {
        QWidget *const window = dialog_parent_->window();
        window->setWindowFlag(Qt::WindowStaysOnTopHint, settings.alwaysOnTop);
        window->show();
    }
    if (settings.language != previousLanguage) {
        emit languageChangeRequested(settings.language);
    }
    completeOnUiThread(requestId, {{QStringLiteral("saved"), true}});
}
