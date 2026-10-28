#include "native_bridge.h"

#include "features/data_conversion/core/emitter/json_emitter.h"
#include "features/data_conversion/core/parser/json_parser.h"
#include "features/db_tool/core/connection_history.h"
#include "features/db_tool/core/database_connection.h"
#include "features/db_tool/core/sqlite_file_access.h"
#include "features/framework/core/application_settings_service.h"
#include "features/image/core/image_processing_service.h"
#include "features/qr_code/core/qr_image.h"
#include "features/regex_tool/core/regex_settings_repository.h"
#include "frontend_operation_service.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonDocument>
#include <QMetaObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>
#include <QStringDecoder>
#include <QtConcurrent/QtConcurrentRun>

#include <algorithm>
#include <array>
#include <cstddef>
#include <exception>

namespace {
constexpr qsizetype IO_CHUNK_BYTES = qsizetype{64} * 1024;
constexpr qint64 MAX_IMAGE_PIXELS = 100'000'000;

QString imagePreviewDataUrl(const QImage &image)
{
    if (image.isNull()) {
        return {};
    }
    QImage preview = image.scaled(QSize(1024, 1024), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    while (true) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        if (!buffer.open(QIODevice::WriteOnly) || !preview.save(&buffer, "PNG")) {
            return {};
        }
        if (bytes.size() <= 768 * 1024) {
            return QStringLiteral("data:image/png;base64,") + QString::fromLatin1(bytes.toBase64());
        }
        if (preview.width() <= 128 || preview.height() <= 128) {
            return {};
        }
        preview = preview.scaled(preview.size() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

bool isSupportedImage(const QImage &image)
{
    return !image.isNull() && image.width() > 0 && image.height() > 0 &&
           static_cast<qint64>(image.width()) * image.height() <= MAX_IMAGE_PIXELS;
}

QString ioErrorCode(QFileDevice::FileError error)
{
    return error == QFileDevice::PermissionsError ? QStringLiteral("PERMISSION_DENIED")
                                                  : QStringLiteral("IO_ERROR");
}
} // namespace

NativeBridge::NativeBridge(QWidget *dialogParent, QObject *parent)
    : QObject(parent), dialog_parent_(dialogParent)
{}

NativeBridge::~NativeBridge()
{
    for (auto &operation : operations_) {
        operation.cancellation->store(true, std::memory_order_relaxed);
        if (operation.dialog != nullptr) {
            operation.dialog->disconnect(this);
            operation.dialog->reject();
        }
        if (operation.watcher != nullptr) {
            operation.watcher->future().waitForFinished();
        }
    }
}

void NativeBridge::openDroppedImageFile(const QString &path)
{
    if (!ImageProcessingService::isReadableImageFile(path)) {
        return;
    }
    dropped_image_path_ = path;
    emit imageDropAvailable();
}

bool NativeBridge::validateRequest(const QString &requestId, const QVariantMap &request)
{
    if (!isValidRequestId(requestId)) {
        fail(QString(), QStringLiteral("INVALID_REQUEST"),
             QStringLiteral("Request ID must be 1 to 80 ASCII letters, digits, '_' or '-'."));
        return false;
    }
    if (request.value(QStringLiteral("contractVersion")).toString() != contract_version_) {
        fail(requestId, QStringLiteral("UNSUPPORTED_VERSION"),
             QStringLiteral("The bridge contract version is not supported."));
        return false;
    }
    const QVariant payloadValue = request.value(QStringLiteral("payload"));
    if (payloadValue.metaType().id() != QMetaType::QVariantMap) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("The request payload must be an object."));
        return false;
    }
    const QVariantMap payload = payloadValue.toMap();
    if (QJsonDocument::fromVariant(payload).toJson(QJsonDocument::Compact).size() >
        MAX_MESSAGE_BYTES) {
        fail(requestId, QStringLiteral("SIZE_LIMIT_EXCEEDED"),
             QStringLiteral("The request exceeds the 2 MiB message limit."));
        return false;
    }
    if (operations_.contains(requestId)) {
        fail(requestId, QStringLiteral("DUPLICATE_REQUEST"),
             QStringLiteral("A request with this ID is already running."));
        return false;
    }
    if (operations_.size() >= MAX_CONCURRENT_OPERATIONS) {
        fail(requestId, QStringLiteral("TOO_MANY_REQUESTS"),
             QStringLiteral("The maximum number of concurrent operations was reached."));
        return false;
    }

    const QString operation = request.value(QStringLiteral("operation")).toString();
    static const QStringList allowedOperations = {
        QStringLiteral("formatJson"),       QStringLiteral("openTextFile"),
        QStringLiteral("saveTextFile"),     QStringLiteral("convertData"),
        QStringLiteral("convertTimestamp"), QStringLiteral("compareText"),
        QStringLiteral("renderMarkdown"),   QStringLiteral("generateQr"),
        QStringLiteral("saveQrImage"),      QStringLiteral("commandData"),
        QStringLiteral("phraseData"),       QStringLiteral("evaluateRegex"),
        QStringLiteral("sendHttpRequest"),  QStringLiteral("regexSettings"),
        QStringLiteral("imageOpen"),        QStringLiteral("imageEdit"),
        QStringLiteral("imageReset"),       QStringLiteral("imageSave"),
        QStringLiteral("imageSplit"),       QStringLiteral("imageTakeDropped"),
        QStringLiteral("databaseHistory"),  QStringLiteral("databaseSelectFile"),
        QStringLiteral("databaseConnect"),  QStringLiteral("databaseDisconnect"),
        QStringLiteral("databaseQuery"),    QStringLiteral("databaseTables"),
        QStringLiteral("getSettings"),      QStringLiteral("saveSettings"),
        QStringLiteral("saveLastUsedTool"), QStringLiteral("copyToClipboard"),
    };
    if (!allowedOperations.contains(operation)) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("The requested operation is not available."));
        return false;
    }
    return true;
}

bool NativeBridge::textPayload(const QString &requestId, const QVariantMap &payload,
                               const QString &sizeError, QString *text)
{
    const QVariant textValue = payload.value(QStringLiteral("text"));
    if (textValue.metaType().id() != QMetaType::QString) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("The text field must be a string."));
        return false;
    }
    *text = textValue.toString();
    if (text->toUtf8().size() > MAX_INPUT_BYTES) {
        fail(requestId, QStringLiteral("SIZE_LIMIT_EXCEEDED"), sizeError);
        return false;
    }
    return true;
}

bool NativeBridge::parseIndentation(const QString &requestId, const QVariantMap &payload,
                                    int *indentation)
{
    const QString value = payload.value(QStringLiteral("indentation")).toString();
    if (value == QStringLiteral("2")) {
        *indentation = 0;
    } else if (value == QStringLiteral("4")) {
        *indentation = 1;
    } else if (value == QStringLiteral("tab")) {
        *indentation = 2;
    } else if (value == QStringLiteral("minified")) {
        *indentation = 3;
    } else {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("Indentation must be 2, 4, tab or minified."));
        return false;
    }
    return true;
}

void NativeBridge::startJsonFormat(const QString &requestId, const QVariantMap &payload)
{
    QString text;
    int indentation = 0;
    if (!textPayload(requestId, payload, QStringLiteral("Input is larger than 1 MiB."), &text) ||
        !parseIndentation(requestId, payload, &indentation)) {
        operations_.remove(requestId);
        return;
    }

    reportProgress(requestId, 5, QStringLiteral("Preparing JSON formatter"));
    startWorker(
        requestId, QStringLiteral("formatJson"),
        [text, indentation](const CancellationToken &token, const ProgressCallback &progress) {
            return formatJson(text, indentation, token, progress);
        });
}

void NativeBridge::startOperation(const QVariantMap &request)
{
    const QString requestId = request.value(QStringLiteral("requestId")).toString();
    if (!validateRequest(requestId, request)) {
        return;
    }

    const QString operation = request.value(QStringLiteral("operation")).toString();
    const QVariantMap payload = request.value(QStringLiteral("payload")).toMap();
    const auto cancellation = std::make_shared<std::atomic_bool>(false);
    operations_.insert(requestId, RunningOperation{operation, cancellation, nullptr, nullptr});
    emit operationStarted(requestId, operation);

    if (operation == QStringLiteral("formatJson")) {
        startJsonFormat(requestId, payload);
        return;
    }
    if (operation == QStringLiteral("convertData") ||
        operation == QStringLiteral("convertTimestamp") ||
        operation == QStringLiteral("compareText") ||
        operation == QStringLiteral("renderMarkdown") ||
        operation == QStringLiteral("generateQr") || operation == QStringLiteral("commandData") ||
        operation == QStringLiteral("phraseData") || operation == QStringLiteral("evaluateRegex") ||
        operation == QStringLiteral("sendHttpRequest")) {
        startToolOperation(requestId, operation, payload);
        return;
    }
    if (operation.startsWith(QStringLiteral("image"))) {
        startImageOperation(requestId, operation, payload);
        return;
    }
    if (operation.startsWith(QStringLiteral("database"))) {
        startDatabaseOperation(requestId, operation, payload);
        return;
    }
    if (operation == QStringLiteral("saveQrImage")) {
        startQrImageSaveOperation(requestId, payload);
        return;
    }
    if (operation == QStringLiteral("regexSettings")) {
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
        static const QStringList boolOptions = {QStringLiteral("global"),
                                                QStringLiteral("caseInsensitive"),
                                                QStringLiteral("multiline"),
                                                QStringLiteral("dotMatchesEverything"),
                                                QStringLiteral("useUnicodeProperties"),
                                                QStringLiteral("extendedPatternSyntax")};
        for (const QString &option : boolOptions) {
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
        settings.dotMatchesEverything =
            options.value(QStringLiteral("dotMatchesEverything")).toBool();
        settings.useUnicodeProperties =
            options.value(QStringLiteral("useUnicodeProperties")).toBool();
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
        return;
    }
    if (operation == QStringLiteral("getSettings")) {
        const ApplicationSettings settings = ApplicationSettingsService().read();
        completeOnUiThread(
            requestId,
            {{QStringLiteral("language"), settings.language},
             {QStringLiteral("showSidebarOnStartup"), settings.showSidemenuOnStartup},
             {QStringLiteral("showLastToolOnStartup"), settings.showLastToolOnStartup},
             {QStringLiteral("lastUsedTool"), settings.lastUsedTool},
             {QStringLiteral("theme"),
              QSettings()
                  .value(QStringLiteral("general/theme"), QStringLiteral("system"))
                  .toString()},
             {QStringLiteral("alwaysOnTop"), settings.alwaysOnTop},
             {QStringLiteral("rememberWindowSize"), settings.rememberWindowSize},
             {QStringLiteral("rememberWindowPosition"), settings.rememberWindowPosition}});
        return;
    }
    if (operation == QStringLiteral("saveSettings")) {
        startSaveSettings(requestId, payload);
        return;
    }
    if (operation == QStringLiteral("saveLastUsedTool")) {
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
        return;
    }
    if (operation == QStringLiteral("copyToClipboard")) {
        if (payload.contains(QStringLiteral("imageDataUrl"))) {
            const QVariant imageValue = payload.value(QStringLiteral("imageDataUrl"));
            const QString imageDataUrl = imageValue.toString();
            const QString prefix = QStringLiteral("data:image/png;base64,");
            if (imageValue.metaType().id() != QMetaType::QString ||
                !imageDataUrl.startsWith(prefix) ||
                imageDataUrl.toUtf8().size() > MAX_MESSAGE_BYTES) {
                fail(requestId, QStringLiteral("INVALID_INPUT"),
                     QStringLiteral("The image clipboard data is invalid."));
                operations_.remove(requestId);
                return;
            }
            const QByteArray imageBytes =
                QByteArray::fromBase64(imageDataUrl.mid(prefix.size()).toLatin1(),
                                       QByteArray::AbortOnBase64DecodingErrors);
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
            if (!dimensions.isValid() ||
                static_cast<qint64>(dimensions.width()) * dimensions.height() > MAX_IMAGE_PIXELS) {
                fail(requestId, QStringLiteral("INVALID_INPUT"),
                     QStringLiteral("The image clipboard data is invalid."));
                operations_.remove(requestId);
                return;
            }
            const QImage image = reader.read();
            if (!isSupportedImage(image)) {
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
        return;
    }
    if (operation == QStringLiteral("saveTextFile")) {
        QString text;
        if (!textPayload(requestId, payload, QStringLiteral("Text to save is larger than 1 MiB."),
                         &text)) {
            operations_.remove(requestId);
            return;
        }
    }

    startFileDialog(requestId, operation, payload);
}

void NativeBridge::startQrImageSaveOperation(const QString &requestId, const QVariantMap &payload)
{
    const auto current = operations_.constFind(requestId);
    if (current == operations_.cend()) {
        return;
    }
    const FrontendOperationService::Result generated = FrontendOperationService::execute(
        QStringLiteral("generateQr"), payload, current->cancellation,
        [this, requestId](int percent, const QString &message) {
            reportProgress(requestId, percent, message);
        });
    if (!generated.errorCode.isEmpty()) {
        fail(requestId, generated.errorCode, generated.errorMessage);
        operations_.remove(requestId);
        return;
    }
    if (generated.cancelled || current->cancellation->load(std::memory_order_relaxed)) {
        operations_.remove(requestId);
        emit operationCancelled(requestId);
        return;
    }

    QImage image;
    try {
        image = QRImage::generate(generated.value.value(QStringLiteral("content")).toString());
    } catch (const std::exception &error) {
        fail(requestId, QStringLiteral("INTERNAL_ERROR"), QString::fromUtf8(error.what()));
        operations_.remove(requestId);
        return;
    }
    if (!isSupportedImage(image)) {
        fail(requestId, QStringLiteral("INTERNAL_ERROR"),
             QStringLiteral("Could not create a QR image."));
        operations_.remove(requestId);
        return;
    }

    auto *dialog =
        new QFileDialog(dialog_parent_, tr("Save QR Code"), QString(), tr("PNG files (*.png)"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->setDefaultSuffix(QStringLiteral("png"));
    dialog->selectFile(QStringLiteral("qr-code.png"));
    auto operation = operations_.find(requestId);
    if (operation == operations_.end()) {
        dialog->deleteLater();
        return;
    }
    operation->dialog = dialog;
    reportProgress(requestId, 10, QStringLiteral("Waiting for file selection"));
    connect(dialog, &QFileDialog::accepted, this, [this, dialog, requestId, image]() {
        auto currentOperation = operations_.find(requestId);
        if (currentOperation == operations_.end()) {
            return;
        }
        QString path = dialog->selectedFiles().value(0);
        currentOperation->dialog = nullptr;
        if (path.isEmpty() || currentOperation->cancellation->load(std::memory_order_relaxed)) {
            operations_.erase(currentOperation);
            emit operationCancelled(requestId);
            return;
        }
        if (!path.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive)) {
            path += QStringLiteral(".png");
        }
        startWorker(
            requestId, QStringLiteral("saveQrImage"),
            [image, path](const CancellationToken &cancellation, const ProgressCallback &progress) {
                if (cancellation->load(std::memory_order_relaxed)) {
                    return OperationResult{{}, {}, {}, true};
                }
                QSaveFile file(path);
                if (!file.open(QIODevice::WriteOnly)) {
                    return OperationResult{
                        {}, ioErrorCode(file.error()), file.errorString(), false};
                }
                progress(40, QStringLiteral("Saving QR image"));
                if (!image.save(&file, "PNG")) {
                    file.cancelWriting();
                    return OperationResult{{},
                                           QStringLiteral("IO_ERROR"),
                                           QStringLiteral("Could not encode the QR image."),
                                           false};
                }
                if (cancellation->load(std::memory_order_relaxed)) {
                    file.cancelWriting();
                    return OperationResult{{}, {}, {}, true};
                }
                if (!file.commit()) {
                    return OperationResult{
                        {}, ioErrorCode(file.error()), file.errorString(), false};
                }
                return OperationResult{
                    {{QStringLiteral("fileName"), QFileInfo(path).fileName()}}, {}, {}, false};
            });
    });
    connect(dialog, &QFileDialog::rejected, this, [this, requestId]() {
        auto currentOperation = operations_.find(requestId);
        if (currentOperation != operations_.end()) {
            operations_.erase(currentOperation);
            emit operationCancelled(requestId);
        }
    });
    dialog->open();
}

void NativeBridge::navigateTo(const QString &route)
{
    static const QStringList allowedRoutes = {QStringLiteral("settings"), QStringLiteral("about"),
                                              QStringLiteral("welcome")};
    if (allowedRoutes.contains(route)) {
        emit navigationRequested(route);
    }
}

void NativeBridge::completeOnUiThread(const QString &requestId, const QVariantMap &result)
{
    operations_.remove(requestId);
    emit operationProgress(requestId, 100, QStringLiteral("Complete"));
    emit operationCompleted(requestId, result);
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

void NativeBridge::startToolOperation(const QString &requestId, const QString &operation,
                                      const QVariantMap &payload)
{
    startWorker(requestId, operation,
                [operation, payload](const CancellationToken &cancellation,
                                     const ProgressCallback &progress) {
                    const FrontendOperationService::Result result =
                        FrontendOperationService::execute(operation, payload, cancellation,
                                                          progress);
                    return OperationResult{result.value, result.errorCode, result.errorMessage,
                                           result.cancelled};
                });
}

NativeBridge::OperationResult NativeBridge::readImageFile(const QString &path,
                                                          const CancellationToken &cancellation,
                                                          const ProgressCallback &progress)
{
    if (cancellation->load(std::memory_order_relaxed)) {
        return OperationResult{{}, {}, {}, true};
    }
    progress(15, QStringLiteral("Reading image"));
    QImageReader reader(path);
    const QSize dimensions = reader.size();
    if (!dimensions.isValid() ||
        static_cast<qint64>(dimensions.width()) * dimensions.height() > MAX_IMAGE_PIXELS) {
        return OperationResult{{},
                               QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                               QStringLiteral("The selected image exceeds the pixel limit."),
                               false};
    }
    const QImage loaded = reader.read();
    if (!isSupportedImage(loaded)) {
        return OperationResult{{},
                               QStringLiteral("INVALID_INPUT"),
                               QStringLiteral("The selected file is not a supported image or "
                                              "exceeds the pixel limit."),
                               false};
    }
    const QString preview = imagePreviewDataUrl(loaded);
    if (preview.isEmpty()) {
        return OperationResult{{},
                               QStringLiteral("INTERNAL_ERROR"),
                               QStringLiteral("Could not create an image preview."),
                               false};
    }
    return OperationResult{{{QStringLiteral("__image"), QVariant::fromValue(loaded)},
                            {QStringLiteral("__originalImage"), QVariant::fromValue(loaded)},
                            {QStringLiteral("__imagePath"), path},
                            {QStringLiteral("imageDataUrl"), preview},
                            {QStringLiteral("fileName"), QFileInfo(path).fileName()},
                            {QStringLiteral("width"), loaded.width()},
                            {QStringLiteral("height"), loaded.height()}},
                           {},
                           {},
                           false};
}

void NativeBridge::startImageOperation(const QString &requestId, const QString &operation,
                                       const QVariantMap &payload)
{
    for (auto it = operations_.cbegin(); it != operations_.cend(); ++it) {
        if (it.key() != requestId && it->operation.startsWith(QStringLiteral("image"))) {
            fail(requestId, QStringLiteral("TOO_MANY_REQUESTS"),
                 QStringLiteral("Finish the current image operation before starting another."));
            operations_.remove(requestId);
            return;
        }
    }

    if (operation == QStringLiteral("imageTakeDropped")) {
        const QString path = std::exchange(dropped_image_path_, QString());
        if (path.isEmpty()) {
            completeOnUiThread(requestId, {{QStringLiteral("found"), false}});
            return;
        }
        startWorker(requestId, operation,
                    [path](const CancellationToken &token, const ProgressCallback &progress) {
                        OperationResult result = NativeBridge::readImageFile(path, token, progress);
                        if (result.errorCode.isEmpty() && !result.cancelled) {
                            result.value.insert(QStringLiteral("found"), true);
                        }
                        return result;
                    });
        return;
    }

    if (operation == QStringLiteral("imageOpen") || operation == QStringLiteral("imageSave") ||
        operation == QStringLiteral("imageSplit")) {
        if (operation != QStringLiteral("imageOpen") && current_image_.isNull()) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("Open an image before using this operation."));
            operations_.remove(requestId);
            return;
        }
        const bool saving = operation == QStringLiteral("imageSave");
        const bool selectingFolder = operation == QStringLiteral("imageSplit");
        const QString title = operation == QStringLiteral("imageOpen") ? tr("Open image")
                              : saving                                 ? tr("Save image")
                                                                       : tr("Select save folder");
        auto *dialog = new QFileDialog(dialog_parent_, title);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        if (selectingFolder) {
            dialog->setFileMode(QFileDialog::Directory);
            dialog->setOption(QFileDialog::ShowDirsOnly, true);
        } else {
            dialog->setNameFilter(
                tr("Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;All files (*)"));
            dialog->setFileMode(saving ? QFileDialog::AnyFile : QFileDialog::ExistingFile);
            if (saving) {
                dialog->setAcceptMode(QFileDialog::AcceptSave);
                dialog->setDefaultSuffix(QStringLiteral("png"));
                dialog->selectFile(current_image_path_.isEmpty()
                                       ? QStringLiteral("image.png")
                                       : QFileInfo(current_image_path_).completeBaseName() +
                                             QStringLiteral("-edited.png"));
            }
        }
        auto it = operations_.find(requestId);
        if (it == operations_.end()) {
            dialog->deleteLater();
            return;
        }
        it->dialog = dialog;
        reportProgress(requestId, 5, QStringLiteral("Waiting for file selection"));
        const QImage image = current_image_;
        const QString originalPath = current_image_path_;
        connect(
            dialog, &QFileDialog::accepted, this,
            [this, dialog, requestId, operation, image, originalPath, payload]() {
                auto current = operations_.find(requestId);
                if (current == operations_.end()) {
                    return;
                }
                const QString path = dialog->selectedFiles().value(0);
                current->dialog = nullptr;
                if (path.isEmpty() || current->cancellation->load(std::memory_order_relaxed)) {
                    operations_.erase(current);
                    emit operationCancelled(requestId);
                    if (!dropped_image_path_.isEmpty()) {
                        emit imageDropAvailable();
                    }
                    return;
                }
                if (operation == QStringLiteral("imageOpen")) {
                    startWorker(
                        requestId, operation,
                        [path](const CancellationToken &token, const ProgressCallback &progress) {
                            return NativeBridge::readImageFile(path, token, progress);
                        });
                    return;
                }
                if (operation == QStringLiteral("imageSave")) {
                    startWorker(requestId, operation,
                                [image, path](const CancellationToken &token,
                                              const ProgressCallback &progress) {
                                    if (token->load(std::memory_order_relaxed)) {
                                        return OperationResult{{}, {}, {}, true};
                                    }
                                    progress(30, QStringLiteral("Saving image"));
                                    if (!ImageProcessingService::save(image, path)) {
                                        return OperationResult{
                                            {},
                                            QStringLiteral("IO_ERROR"),
                                            QStringLiteral("Could not save the image."),
                                            false};
                                    }
                                    return OperationResult{
                                        {{QStringLiteral("fileName"), QFileInfo(path).fileName()}},
                                        {},
                                        {},
                                        false};
                                });
                    return;
                }

                bool horizontalValid = false;
                bool verticalValid = false;
                bool cellWidthValid = false;
                bool cellHeightValid = false;
                ImageSplitOptions options;
                const QString mode = payload.value(QStringLiteral("mode")).toString();
                if (mode == QStringLiteral("cell-size")) {
                    options.mode = ImageSplitMode::CellSize;
                    options.cellWidth =
                        payload.value(QStringLiteral("cellWidth")).toInt(&cellWidthValid);
                    options.cellHeight =
                        payload.value(QStringLiteral("cellHeight")).toInt(&cellHeightValid);
                    if (!cellWidthValid || !cellHeightValid || options.cellWidth < 1 ||
                        options.cellHeight < 1 || options.cellWidth > image.width() ||
                        options.cellHeight > image.height()) {
                        fail(requestId, QStringLiteral("INVALID_INPUT"),
                             QStringLiteral("Cell dimensions must fit within the image."));
                        operations_.remove(requestId);
                        return;
                    }
                } else {
                    options.mode = ImageSplitMode::DivisionCount;
                    options.horizontalDivisions =
                        payload.value(QStringLiteral("horizontalDivisions"))
                            .toInt(&horizontalValid);
                    options.verticalDivisions =
                        payload.value(QStringLiteral("verticalDivisions")).toInt(&verticalValid);
                    if (!horizontalValid || !verticalValid || options.horizontalDivisions < 1 ||
                        options.verticalDivisions < 1 ||
                        options.horizontalDivisions > image.width() ||
                        options.verticalDivisions > image.height()) {
                        fail(requestId, QStringLiteral("INVALID_INPUT"),
                             QStringLiteral("Division counts must fit within the image."));
                        operations_.remove(requestId);
                        return;
                    }
                }
                options.ignoreRemainders =
                    payload.value(QStringLiteral("ignoreRemainders")).toBool();
                startWorker(
                    requestId, operation,
                    [image, originalPath, path, options](const CancellationToken &token,
                                                         const ProgressCallback &progress) {
                        if (token->load(std::memory_order_relaxed)) {
                            return OperationResult{{}, {}, {}, true};
                        }
                        progress(20, QStringLiteral("Splitting image"));
                        if (!ImageProcessingService::saveDividedImages(image, originalPath, path,
                                                                       options)) {
                            return OperationResult{{},
                                                   QStringLiteral("IO_ERROR"),
                                                   QStringLiteral("Could not save divided images."),
                                                   false};
                        }
                        return OperationResult{{{QStringLiteral("saved"), true}}, {}, {}, false};
                    });
            });
        connect(dialog, &QFileDialog::rejected, this, [this, requestId]() {
            auto current = operations_.find(requestId);
            if (current != operations_.end()) {
                operations_.erase(current);
                emit operationCancelled(requestId);
            }
        });
        dialog->open();
        return;
    }

    if (operation == QStringLiteral("imageReset")) {
        if (original_image_.isNull()) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("Open an image before resetting it."));
            operations_.remove(requestId);
            return;
        }
        const QImage restored = original_image_;
        startWorker(requestId, operation,
                    [restored](const CancellationToken &token, const ProgressCallback &progress) {
                        if (token->load(std::memory_order_relaxed)) {
                            return OperationResult{{}, {}, {}, true};
                        }
                        const QString preview = imagePreviewDataUrl(restored);
                        return OperationResult{
                            {{QStringLiteral("__image"), QVariant::fromValue(restored)},
                             {QStringLiteral("imageDataUrl"), preview},
                             {QStringLiteral("width"), restored.width()},
                             {QStringLiteral("height"), restored.height()}},
                            {},
                            {},
                            false};
                    });
        return;
    }

    if (operation != QStringLiteral("imageEdit") || current_image_.isNull()) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("Open an image before editing it."));
        operations_.remove(requestId);
        return;
    }
    const QImage source = current_image_;
    startWorker(
        requestId, operation,
        [source, payload](const CancellationToken &token, const ProgressCallback &progress) {
            if (token->load(std::memory_order_relaxed)) {
                return OperationResult{{}, {}, {}, true};
            }
            QImage changed;
            const QString action = payload.value(QStringLiteral("action")).toString();
            if (action == QStringLiteral("rotate-left")) {
                changed = ImageProcessingService::rotateLeft(source);
            } else if (action == QStringLiteral("rotate-right")) {
                changed = ImageProcessingService::rotateRight(source);
            } else if (action == QStringLiteral("flip-horizontal")) {
                changed = ImageProcessingService::flip(source, ImageFlipDirection::Horizontal);
            } else if (action == QStringLiteral("flip-vertical")) {
                changed = ImageProcessingService::flip(source, ImageFlipDirection::Vertical);
            } else if (action == QStringLiteral("resize-size")) {
                bool widthValid = false;
                bool heightValid = false;
                const int width = payload.value(QStringLiteral("width")).toInt(&widthValid);
                const int height = payload.value(QStringLiteral("height")).toInt(&heightValid);
                if (!widthValid || !heightValid || width < 1 || height < 1 || width > 32768 ||
                    height > 32768 || static_cast<qint64>(width) * height > MAX_IMAGE_PIXELS) {
                    return OperationResult{
                        {},
                        QStringLiteral("INVALID_INPUT"),
                        QStringLiteral("Image dimensions exceed the allowed size."),
                        false};
                }
                changed = ImageProcessingService::resizeBySize(
                    source, width, height,
                    payload.value(QStringLiteral("keepAspectRatio")).toBool(),
                    payload.value(QStringLiteral("smoothScaling")).toBool());
            } else if (action == QStringLiteral("resize-scale")) {
                bool horizontalValid = false;
                bool verticalValid = false;
                const double horizontal =
                    payload.value(QStringLiteral("horizontalPercent")).toDouble(&horizontalValid);
                const double vertical =
                    payload.value(QStringLiteral("verticalPercent")).toDouble(&verticalValid);
                if (!horizontalValid || !verticalValid || horizontal < 0.1 || vertical < 0.1 ||
                    horizontal > 1000.0 || vertical > 1000.0 ||
                    static_cast<double>(source.width()) * horizontal *
                            static_cast<double>(source.height()) * vertical / 10000.0 >
                        static_cast<double>(MAX_IMAGE_PIXELS)) {
                    return OperationResult{{},
                                           QStringLiteral("INVALID_INPUT"),
                                           QStringLiteral("Image scale exceeds the allowed size."),
                                           false};
                }
                changed = ImageProcessingService::resizeByScale(
                    source, horizontal, vertical,
                    payload.value(QStringLiteral("keepAspectRatio")).toBool(),
                    payload.value(QStringLiteral("smoothScaling")).toBool());
            } else if (action == QStringLiteral("transparent")) {
                bool xValid = false;
                bool yValid = false;
                const int x = payload.value(QStringLiteral("x")).toInt(&xValid);
                const int y = payload.value(QStringLiteral("y")).toInt(&yValid);
                const double tolerance = payload.value(QStringLiteral("tolerance")).toDouble();
                const double transparency =
                    payload.value(QStringLiteral("transparency")).toDouble();
                if (!xValid || !yValid || !source.rect().contains(x, y) || tolerance < 0.0 ||
                    tolerance > 1.0 || transparency < 0.0 || transparency > 1.0) {
                    return OperationResult{
                        {},
                        QStringLiteral("INVALID_INPUT"),
                        QStringLiteral("Choose a point and valid transparency settings."),
                        false};
                }
                QColor::Spec spec = QColor::Spec::Rgb;
                const QString colorMode = payload.value(QStringLiteral("colorMode")).toString();
                if (colorMode == QStringLiteral("hsl")) {
                    spec = QColor::Spec::Hsl;
                } else if (colorMode == QStringLiteral("hsv")) {
                    spec = QColor::Spec::Hsv;
                } else if (colorMode != QStringLiteral("rgb")) {
                    return OperationResult{{},
                                           QStringLiteral("INVALID_INPUT"),
                                           QStringLiteral("Color mode is not supported."),
                                           false};
                }
                changed = ImageProcessingService::makeTransparent(
                    source, QPoint(x, y), source.pixelColor(x, y), spec, tolerance,
                    ImageProcessingService::alphaForTransparency(transparency),
                    payload.value(QStringLiteral("contiguousOnly")).toBool());
            } else {
                return OperationResult{{},
                                       QStringLiteral("INVALID_INPUT"),
                                       QStringLiteral("Image edit action is not supported."),
                                       false};
            }
            if (!isSupportedImage(changed)) {
                return OperationResult{{},
                                       QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                                       QStringLiteral("The edited image exceeds the pixel limit."),
                                       false};
            }
            if (token->load(std::memory_order_relaxed)) {
                return OperationResult{{}, {}, {}, true};
            }
            progress(90, QStringLiteral("Preparing image preview"));
            const QString preview = imagePreviewDataUrl(changed);
            if (preview.isEmpty()) {
                return OperationResult{{},
                                       QStringLiteral("INTERNAL_ERROR"),
                                       QStringLiteral("Could not create an image preview."),
                                       false};
            }
            return OperationResult{{{QStringLiteral("__image"), QVariant::fromValue(changed)},
                                    {QStringLiteral("imageDataUrl"), preview},
                                    {QStringLiteral("width"), changed.width()},
                                    {QStringLiteral("height"), changed.height()}},
                                   {},
                                   {},
                                   false};
        });
}

void NativeBridge::startDatabaseOperation(const QString &requestId, const QString &operation,
                                          const QVariantMap &payload)
{
    if (operation == QStringLiteral("databaseHistory")) {
        const QSettings settings;
        const QList<QJsonObject> stored = ConnectionHistory::read(settings);
        QVariantList history;
        for (int index = 0; index < stored.size(); ++index) {
            const QJsonObject item = stored.at(index);
            const QString type = item.value(QStringLiteral("type")).toString();
            const QString database = item.value(QStringLiteral("database")).toString();
            const QString displayName = item.value(QStringLiteral("displayName")).toString();
            history.append(QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("type"), type},
                {QStringLiteral("host"), item.value(QStringLiteral("host")).toString()},
                {QStringLiteral("database"),
                 type == QStringLiteral("QSQLITE") ? QFileInfo(database).fileName() : database},
                {QStringLiteral("username"), item.value(QStringLiteral("username")).toString()},
                {QStringLiteral("displayName"), displayName}});
        }
        completeOnUiThread(
            requestId,
            {{QStringLiteral("history"), history},
             {QStringLiteral("selectedFileName"), QFileInfo(selected_sqlite_path_).fileName()},
             {QStringLiteral("connected"), !database_connection_.isEmpty()}});
        return;
    }
    if (operation == QStringLiteral("databaseDisconnect")) {
        database_connection_.clear();
        sqlite_file_access_.reset();
        selected_sqlite_path_.clear();
        completeOnUiThread(requestId, {{QStringLiteral("disconnected"), true}});
        return;
    }
    if (operation == QStringLiteral("databaseSelectFile")) {
        auto *dialog =
            new QFileDialog(dialog_parent_, tr("Select Database File"), QString(),
                            tr("SQLite Database (*.db *.sqlite *.sqlite3);;All Files (*)"));
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setFileMode(QFileDialog::ExistingFile);
        auto it = operations_.find(requestId);
        if (it == operations_.end()) {
            dialog->deleteLater();
            return;
        }
        it->dialog = dialog;
        connect(dialog, &QFileDialog::accepted, this, [this, dialog, requestId]() {
            auto current = operations_.find(requestId);
            if (current == operations_.end()) {
                return;
            }
            const QString path = dialog->selectedFiles().value(0);
            current->dialog = nullptr;
            if (path.isEmpty() ||
                QFileInfo(path).suffix().compare(QStringLiteral("db"), Qt::CaseInsensitive) != 0 &&
                    QFileInfo(path).suffix().compare(QStringLiteral("sqlite"),
                                                     Qt::CaseInsensitive) != 0 &&
                    QFileInfo(path).suffix().compare(QStringLiteral("sqlite3"),
                                                     Qt::CaseInsensitive) != 0) {
                operations_.erase(current);
                emit operationCancelled(requestId);
                return;
            }
            std::unique_ptr<SQLiteFileAccess> access = SQLiteFileAccess::fromFilePath(path);
            if (!access) {
                fail(requestId, QStringLiteral("PERMISSION_DENIED"),
                     QStringLiteral("Could not access the selected database file."));
                operations_.remove(requestId);
                return;
            }
            sqlite_file_access_ = std::shared_ptr<SQLiteFileAccess>(std::move(access));
            selected_sqlite_path_ = sqlite_file_access_->filePath();
            completeOnUiThread(requestId, {{QStringLiteral("selected"), true},
                                           {QStringLiteral("fileName"),
                                            QFileInfo(selected_sqlite_path_).fileName()}});
        });
        connect(dialog, &QFileDialog::rejected, this, [this, requestId]() {
            auto current = operations_.find(requestId);
            if (current != operations_.end()) {
                operations_.erase(current);
                emit operationCancelled(requestId);
            }
        });
        dialog->open();
        return;
    }
    if (operation == QStringLiteral("databaseConnect")) {
        QVariantMap connection;
        int historyIndex = -1;
        bool historyIndexValid = false;
        if (payload.contains(QStringLiteral("historyIndex"))) {
            historyIndex = payload.value(QStringLiteral("historyIndex")).toInt(&historyIndexValid);
            const QSettings settings;
            const QList<QJsonObject> stored = ConnectionHistory::read(settings);
            if (!historyIndexValid || historyIndex < 0 || historyIndex >= stored.size()) {
                fail(requestId, QStringLiteral("INVALID_INPUT"),
                     QStringLiteral("Select a valid saved connection."));
                operations_.remove(requestId);
                return;
            }
            connection = stored.at(historyIndex).toVariantMap();
            const QString type = connection.value(QStringLiteral("type")).toString();
            if (type == QStringLiteral("QSQLITE")) {
                const QByteArray bookmark = QByteArray::fromBase64(
                    connection.value(QStringLiteral("securityScopedBookmark"))
                        .toString()
                        .toLatin1());
                std::unique_ptr<SQLiteFileAccess> restored =
                    bookmark.isEmpty() ? nullptr : SQLiteFileAccess::fromBookmark(bookmark);
                if (!restored) {
                    restored = SQLiteFileAccess::fromFilePath(
                        connection.value(QStringLiteral("database")).toString());
                }
                if (!restored) {
                    fail(
                        requestId, QStringLiteral("PERMISSION_DENIED"),
                        QStringLiteral("The saved SQLite file permission is no longer available."));
                    operations_.remove(requestId);
                    return;
                }
                sqlite_file_access_ = std::shared_ptr<SQLiteFileAccess>(std::move(restored));
                selected_sqlite_path_ = sqlite_file_access_->filePath();
                connection.insert(QStringLiteral("database"), selected_sqlite_path_);
            } else {
                connection.insert(QStringLiteral("password"),
                                  payload.value(QStringLiteral("password")).toString());
            }
        } else {
            connection = payload;
            const QString type = connection.value(QStringLiteral("type")).toString();
            if (type == QStringLiteral("QSQLITE") ||
                (type.isEmpty() && !selected_sqlite_path_.isEmpty())) {
                if (!sqlite_file_access_ || selected_sqlite_path_.isEmpty()) {
                    fail(requestId, QStringLiteral("INVALID_INPUT"),
                         QStringLiteral("Choose a SQLite file before connecting."));
                    operations_.remove(requestId);
                    return;
                }
                connection.insert(QStringLiteral("type"), QStringLiteral("QSQLITE"));
                connection.insert(QStringLiteral("database"), selected_sqlite_path_);
                connection.insert(QStringLiteral("host"), QString());
                connection.insert(QStringLiteral("username"), QString());
                connection.insert(QStringLiteral("password"), QString());
                connection.insert(
                    QStringLiteral("displayName"),
                    QStringLiteral("SQLite: %1").arg(QFileInfo(selected_sqlite_path_).fileName()));
            }
        }
        const QString type = connection.value(QStringLiteral("type")).toString();
        if (type != QStringLiteral("QSQLITE") && type != QStringLiteral("QMYSQL") &&
            type != QStringLiteral("QPSQL")) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("Choose SQLite, MySQL or PostgreSQL."));
            operations_.remove(requestId);
            return;
        }
        const auto access = sqlite_file_access_;
        startWorker(
            requestId, operation,
            [connection, access](const CancellationToken &token, const ProgressCallback &progress) {
                Q_UNUSED(access);
                const FrontendOperationService::Result result = FrontendOperationService::execute(
                    QStringLiteral("databaseConnect"), connection, token, progress);
                if (!result.errorCode.isEmpty() || result.cancelled) {
                    return OperationResult{result.value, result.errorCode, result.errorMessage,
                                           result.cancelled};
                }
                QVariantMap value = result.value;
                value.insert(QStringLiteral("__connection"), connection);
                return OperationResult{value, {}, {}, false};
            });
        return;
    }
    if (operation == QStringLiteral("databaseQuery") ||
        operation == QStringLiteral("databaseTables")) {
        if (database_connection_.isEmpty()) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("Connect to a database first."));
            operations_.remove(requestId);
            return;
        }
        QVariantMap requestPayload = payload;
        requestPayload.insert(QStringLiteral("connection"), database_connection_);
        const auto access = sqlite_file_access_;
        startWorker(requestId, operation,
                    [operation, requestPayload, access](const CancellationToken &token,
                                                        const ProgressCallback &progress) {
                        Q_UNUSED(access);
                        const FrontendOperationService::Result result =
                            FrontendOperationService::execute(operation, requestPayload, token,
                                                              progress);
                        return OperationResult{result.value, result.errorCode, result.errorMessage,
                                               result.cancelled};
                    });
        return;
    }
    fail(requestId, QStringLiteral("INVALID_INPUT"),
         QStringLiteral("The database operation is not available."));
    operations_.remove(requestId);
}

void NativeBridge::openDroppedSQLiteFile(const QString &path)
{
    const QFileInfo file(path);
    const QString suffix = file.suffix().toLower();
    if (!file.isFile() || (suffix != QStringLiteral("db") && suffix != QStringLiteral("sqlite") &&
                           suffix != QStringLiteral("sqlite3"))) {
        return;
    }
    std::unique_ptr<SQLiteFileAccess> access =
        SQLiteFileAccess::fromFilePath(file.absoluteFilePath());
    if (!access) {
        return;
    }
    sqlite_file_access_ = std::shared_ptr<SQLiteFileAccess>(std::move(access));
    selected_sqlite_path_ = sqlite_file_access_->filePath();
    database_connection_.clear();
    emit navigationRequested(QStringLiteral("db"));
}

void NativeBridge::cancelOperation(const QString &requestId)
{
    auto it = operations_.find(requestId);
    if (it == operations_.end()) {
        return;
    }

    it->cancellation->store(true, std::memory_order_relaxed);
    if (it->dialog != nullptr) {
        it->dialog->reject();
    } else {
        reportProgress(requestId, 0, QStringLiteral("Cancellation requested"));
    }
}

void NativeBridge::fail(const QString &requestId, const QString &code, const QString &message)
{
    emit operationFailed(requestId,
                         {{QStringLiteral("code"), code}, {QStringLiteral("message"), message}});
}

void NativeBridge::startWorker(const QString &requestId, const QString &operation, Worker worker)
{
    auto it = operations_.find(requestId);
    if (it == operations_.end()) {
        return;
    }
    if (it->cancellation->load(std::memory_order_relaxed)) {
        operations_.erase(it);
        emit operationCancelled(requestId);
        return;
    }

    const CancellationToken cancellation = it->cancellation;
    const ProgressCallback progress = [this, requestId](int percent, const QString &message) {
        emit operationProgress(requestId, percent, message);
    };
    auto *watcher = new QFutureWatcher<OperationResult>(this);
    it->operation = operation;
    it->watcher = watcher;

    connect(watcher, &QFutureWatcher<OperationResult>::finished, this,
            [this, requestId, watcher, operation]() {
                const auto it = operations_.find(requestId);
                if (it == operations_.end() || it->watcher != watcher) {
                    watcher->deleteLater();
                    return;
                }

                const OperationResult result = watcher->result();
                operations_.erase(it);
                watcher->deleteLater();

                if (result.cancelled) {
                    emit operationCancelled(requestId);
                } else if (!result.errorCode.isEmpty()) {
                    fail(requestId, result.errorCode, result.errorMessage);
                } else {
                    QVariantMap publicResult = result.value;
                    const QVariant imageValue = publicResult.take(QStringLiteral("__image"));
                    if (imageValue.metaType().id() == QMetaType::QImage) {
                        current_image_ = imageValue.value<QImage>();
                    }
                    const QVariant originalImageValue =
                        publicResult.take(QStringLiteral("__originalImage"));
                    if (originalImageValue.metaType().id() == QMetaType::QImage) {
                        original_image_ = originalImageValue.value<QImage>();
                    }
                    const QString imagePath =
                        publicResult.take(QStringLiteral("__imagePath")).toString();
                    if (!imagePath.isEmpty()) {
                        current_image_path_ = imagePath;
                    }
                    const QVariantMap connection =
                        publicResult.take(QStringLiteral("__connection")).toMap();
                    if (operation == QStringLiteral("databaseConnect") && !connection.isEmpty()) {
                        database_connection_ = connection;
                        const QString type = connection.value(QStringLiteral("type")).toString();
                        const QString typeText = type == QStringLiteral("QSQLITE") ? tr("SQLite")
                                                 : type == QStringLiteral("QMYSQL")
                                                     ? tr("MySQL")
                                                     : tr("PostgreSQL");
                        const DatabaseParameters parameters{
                            type, connection.value(QStringLiteral("host")).toString(),
                            connection.value(QStringLiteral("database")).toString(),
                            connection.value(QStringLiteral("username")).toString(),
                            connection.value(QStringLiteral("password")).toString()};
                        const QByteArray bookmark = sqlite_file_access_
                                                        ? sqlite_file_access_->bookmarkData()
                                                        : QByteArray{};
                        const QJsonObject historyEntry =
                            DatabaseConnection::connectionInfo(parameters, typeText, bookmark);
                        QSettings settings;
                        ConnectionHistory::add(settings, historyEntry);
                    }
                    emit operationProgress(requestId, 100, QStringLiteral("Complete"));
                    emit operationCompleted(requestId, publicResult);
                }
                if (!dropped_image_path_.isEmpty()) {
                    emit imageDropAvailable();
                }
            });

    watcher->setFuture(QtConcurrent::run([worker = std::move(worker), cancellation, progress]() {
        return worker(cancellation, progress);
    }));
}

void NativeBridge::reportProgress(const QString &requestId, int percent, const QString &message)
{
    emit operationProgress(requestId, percent, message);
}

void NativeBridge::startFileDialog(const QString &requestId, const QString &operation,
                                   const QVariantMap &payload)
{
    const bool saving = operation == QStringLiteral("saveTextFile");
    const QString fileType = payload.value(QStringLiteral("fileType")).toString();
    QString filter = tr("Text files (*.txt);;All files (*)");
    QString suffix = QStringLiteral("txt");
    if (fileType == QStringLiteral("data")) {
        filter = tr("Data files (*.json *.yaml *.yml *.toml);;All files (*)");
        suffix = QStringLiteral("json");
    } else if (fileType == QStringLiteral("markdown")) {
        filter = tr("Markdown files (*.md *.markdown);;All files (*)");
        suffix = QStringLiteral("md");
    } else if (fileType == QStringLiteral("html")) {
        filter = tr("HTML files (*.html *.htm);;All files (*)");
        suffix = QStringLiteral("html");
    }
    auto *dialog = new QFileDialog(
        dialog_parent_, saving ? tr("Save text file") : tr("Open text file"), QString(), filter);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setFileMode(saving ? QFileDialog::AnyFile : QFileDialog::ExistingFile);
    if (saving) {
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        const QString suggested =
            safeSuggestedName(payload.value(QStringLiteral("suggestedName")).toString());
        dialog->setDefaultSuffix(suffix);
        dialog->selectFile(suggested.isEmpty() ? QStringLiteral("output.%1").arg(suffix)
                                               : suggested);
    }

    auto it = operations_.find(requestId);
    if (it == operations_.end()) {
        dialog->deleteLater();
        return;
    }
    it->dialog = dialog;
    reportProgress(requestId, 5, QStringLiteral("Waiting for file selection"));

    connect(dialog, &QFileDialog::accepted, this, [this, dialog, requestId, operation, payload]() {
        auto current = operations_.find(requestId);
        if (current == operations_.end()) {
            return;
        }
        const QString path = dialog->selectedFiles().value(0);
        current->dialog = nullptr;
        if (path.isEmpty() || current->cancellation->load(std::memory_order_relaxed)) {
            operations_.erase(current);
            emit operationCancelled(requestId);
            return;
        }

        if (operation == QStringLiteral("openTextFile")) {
            startWorker(requestId, operation,
                        [path](const CancellationToken &token, const ProgressCallback &progress) {
                            return NativeBridge::readTextFile(path, token, progress);
                        });
            return;
        }

        const QString text = payload.value(QStringLiteral("text")).toString();
        startWorker(requestId, operation,
                    [path, text](const CancellationToken &token, const ProgressCallback &progress) {
                        return NativeBridge::saveTextFile(path, text, token, progress);
                    });
    });
    connect(dialog, &QFileDialog::rejected, this, [this, requestId]() {
        auto current = operations_.find(requestId);
        if (current != operations_.end()) {
            operations_.erase(current);
            emit operationCancelled(requestId);
        }
    });
    dialog->open();
}

NativeBridge::OperationResult NativeBridge::formatJson(const QString &text, int indentation,
                                                       const CancellationToken &cancellation,
                                                       const ProgressCallback &progress)
{
    if (cancellation->load(std::memory_order_relaxed)) {
        return OperationResult{{}, {}, {}, true};
    }
    progress(25, QStringLiteral("Parsing JSON"));
    const JsonParser::ParseResult parsed = JsonParser().tryParse(text);
    if (!parsed.success) {
        return OperationResult{
            {}, QStringLiteral("INVALID_INPUT"), parsed.errors.join(QStringLiteral("\n")), false};
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        return OperationResult{{}, {}, {}, true};
    }

    static constexpr std::array<DataConversion::Indentation, 4> INDENTATIONS = {
        DataConversion::Indentation::SPACES_2,
        DataConversion::Indentation::SPACES_4,
        DataConversion::Indentation::TABS,
        DataConversion::Indentation::MINIFIED,
    };
    progress(80, QStringLiteral("Formatting JSON"));
    JsonEmitter emitter;
    const BasicEmitter::EmitResult emitted =
        emitter.emitQString(parsed.data, INDENTATIONS.at(static_cast<std::size_t>(indentation)));
    if (!emitted.error.isEmpty()) {
        return OperationResult{{}, QStringLiteral("INVALID_INPUT"), emitted.error, false};
    }
    if (emitted.text.toUtf8().size() > MAX_OUTPUT_BYTES) {
        return OperationResult{{},
                               QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                               QStringLiteral("Formatted output is larger than 4 MiB."),
                               false};
    }
    return OperationResult{{{QStringLiteral("text"), emitted.text}}, {}, {}, false};
}

NativeBridge::OperationResult NativeBridge::readTextFile(const QString &path,
                                                         const CancellationToken &cancellation,
                                                         const ProgressCallback &progress)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
    }
    if (file.size() > MAX_INPUT_BYTES) {
        return OperationResult{{},
                               QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                               QStringLiteral("Selected file is larger than 1 MiB."),
                               false};
    }

    QByteArray bytes;
    const qint64 totalBytes = file.size();
    while (!file.atEnd()) {
        if (cancellation->load(std::memory_order_relaxed)) {
            return OperationResult{{}, {}, {}, true};
        }
        const QByteArray chunk = file.read(IO_CHUNK_BYTES);
        if (chunk.isEmpty() && file.error() != QFileDevice::NoError) {
            return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
        }
        bytes.append(chunk);
        if (bytes.size() > MAX_INPUT_BYTES) {
            return OperationResult{{},
                                   QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                                   QStringLiteral("Selected file is larger than 1 MiB."),
                                   false};
        }
        const int percent =
            totalBytes > 0
                ? 10 + static_cast<int>(static_cast<qint64>(bytes.size()) * 80 / totalBytes)
                : 90;
        progress(std::min(percent, 90), QStringLiteral("Reading file"));
    }

    QStringDecoder decoder(QStringDecoder::Utf8, QStringConverter::Flag::Stateless);
    const QString text = decoder(bytes);
    if (decoder.hasError()) {
        return OperationResult{{},
                               QStringLiteral("INVALID_ENCODING"),
                               QStringLiteral("The selected file is not valid UTF-8."),
                               false};
    }
    return OperationResult{
        {{QStringLiteral("text"), text}, {QStringLiteral("fileName"), QFileInfo(path).fileName()}},
        {},
        {},
        false};
}

NativeBridge::OperationResult NativeBridge::saveTextFile(const QString &path, const QString &text,
                                                         const CancellationToken &cancellation,
                                                         const ProgressCallback &progress)
{
    const QByteArray bytes = text.toUtf8();
    if (bytes.size() > MAX_INPUT_BYTES) {
        return OperationResult{{},
                               QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                               QStringLiteral("Text to save is larger than 1 MiB."),
                               false};
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
    }
    qint64 writtenBytes = 0;
    while (writtenBytes < bytes.size()) {
        if (cancellation->load(std::memory_order_relaxed)) {
            file.cancelWriting();
            return OperationResult{{}, {}, {}, true};
        }
        const qint64 remainingBytes = bytes.size() - writtenBytes;
        const qint64 chunkSize = std::min<qint64>(IO_CHUNK_BYTES, remainingBytes);
        const qint64 written = file.write(bytes.constData() + writtenBytes, chunkSize);
        if (written <= 0) {
            file.cancelWriting();
            return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
        }
        writtenBytes += written;
        const int percent =
            bytes.isEmpty() ? 90 : 10 + static_cast<int>(writtenBytes * 80 / bytes.size());
        progress(std::min(percent, 90), QStringLiteral("Writing file"));
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        file.cancelWriting();
        return OperationResult{{}, {}, {}, true};
    }
    if (!file.commit()) {
        return OperationResult{{}, ioErrorCode(file.error()), file.errorString(), false};
    }

    return OperationResult{{{QStringLiteral("fileName"), QFileInfo(path).fileName()},
                            {QStringLiteral("bytesWritten"), writtenBytes}},
                           {},
                           {},
                           false};
}

bool NativeBridge::isValidRequestId(const QString &requestId)
{
    static const QRegularExpression pattern(QStringLiteral("^[A-Za-z0-9_-]{1,80}$"));
    return pattern.match(requestId).hasMatch();
}

QString NativeBridge::safeSuggestedName(const QString &name)
{
    QString safeName = name;
    safeName.replace(QRegularExpression(QStringLiteral("[/\\\\]")), QStringLiteral("_"));
    safeName.remove(QStringLiteral(".."));
    safeName = safeName.trimmed();
    if (safeName.isEmpty()) {
        return QStringLiteral("output.txt");
    }
    return safeName.left(128);
}
