#include "features/data_conversion/core/emitter/json_emitter.h"
#include "features/data_conversion/core/parser/json_parser.h"
#include "features/qr_code/core/qr_image.h"
#include "features/web_frontend/core/operations/frontend_operation_service.h"
#include "features/web_frontend/gui/bridge/bridge_io.h"
#include "features/web_frontend/gui/native_bridge.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QSaveFile>

#include <exception>

namespace {
constexpr qint64 MAX_QR_IMAGE_PIXELS = 100'000'000;

bool isSupportedImage(const QImage &image)
{
    return !image.isNull() && image.width() > 0 && image.height() > 0 &&
           static_cast<qint64>(image.width()) * image.height() <= MAX_QR_IMAGE_PIXELS;
}
} // namespace

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

// NOLINTNEXTLINE(readability-function-size) -- dialog and save worker share one operation lifetime.
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
                        {}, frontendIoErrorCode(file.error()), file.errorString(), false};
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
                        {}, frontendIoErrorCode(file.error()), file.errorString(), false};
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
