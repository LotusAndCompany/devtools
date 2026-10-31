#include "features/image/core/image_processing_service.h"
#include "features/web_frontend/gui/handlers/image_state_handler.h"
#include "features/web_frontend/gui/native_bridge.h"

#include <QBuffer>
#include <QColor>
#include <QFileDialog>
#include <QFileInfo>
#include <QIODevice>
#include <QImageReader>
#include <QMetaType>
#include <QSize>

#include <algorithm>

namespace {
constexpr qint64 MAX_IMAGE_PIXELS = 100'000'000;
constexpr qsizetype MAX_IMAGE_PREVIEW_BYTES = qsizetype{768} * 1024;

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
        if (bytes.size() <= MAX_IMAGE_PREVIEW_BYTES) {
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
} // namespace

void NativeBridge::openDroppedImageFile(const QString &path)
{
    if (!image_state_->queueDroppedImage(path)) {
        return;
    }
    emit imageDropAvailable();
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

// NOLINTNEXTLINE(readability-function-size) -- operation routes share dialog and worker state.
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
        const QString path = image_state_->takeDroppedImagePath();
        if (path.isEmpty()) {
            completeOnUiThread(requestId, {{QStringLiteral("found"), false}});
            return;
        }
        startWorker(
            requestId, operation,
            [path](const CancellationToken &token, const ProgressCallback &progress) {
                OperationResult result = NativeBridge::readImageFile(path, token, progress);
                if (result.errorCode.isEmpty() && !result.cancelled) {
                    result.value.insert(QStringLiteral("found"), true);
                }
                return result;
            },
            [this](QVariantMap &result) { image_state_->applyWorkerResult(result); });
        return;
    }

    if (operation == QStringLiteral("imageOpen") || operation == QStringLiteral("imageSave") ||
        operation == QStringLiteral("imageSplit")) {
        if (operation != QStringLiteral("imageOpen") && !image_state_->hasCurrentImage()) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("Open an image before using this operation."));
            operations_.remove(requestId);
            return;
        }
        const bool saving = operation == QStringLiteral("imageSave");
        const bool selectingFolder = operation == QStringLiteral("imageSplit");
        QString title = tr("Select save folder");
        if (operation == QStringLiteral("imageOpen")) {
            title = tr("Open image");
        } else if (saving) {
            title = tr("Save image");
        }
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
                dialog->selectFile(
                    image_state_->currentImagePath().isEmpty()
                        ? QStringLiteral("image.png")
                        : QFileInfo(image_state_->currentImagePath()).completeBaseName() +
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
        const QImage image = image_state_->currentImage();
        const QString originalPath = image_state_->currentImagePath();
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
                    if (image_state_->hasDroppedImage()) {
                        emit imageDropAvailable();
                    }
                    return;
                }
                if (operation == QStringLiteral("imageOpen")) {
                    startWorker(
                        requestId, operation,
                        [path](const CancellationToken &token, const ProgressCallback &progress) {
                            return NativeBridge::readImageFile(path, token, progress);
                        },
                        [this](QVariantMap &result) { image_state_->applyWorkerResult(result); });
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
                if (image_state_->hasDroppedImage()) {
                    emit imageDropAvailable();
                }
            }
        });
        dialog->open();
        return;
    }

    if (operation == QStringLiteral("imageReset")) {
        if (image_state_->originalImage().isNull()) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("Open an image before resetting it."));
            operations_.remove(requestId);
            return;
        }
        const QImage restored = image_state_->originalImage();
        startWorker(
            requestId, operation,
            [restored](const CancellationToken &token, const ProgressCallback &progress) {
                Q_UNUSED(progress);
                if (token->load(std::memory_order_relaxed)) {
                    return OperationResult{{}, {}, {}, true};
                }
                const QString preview = imagePreviewDataUrl(restored);
                return OperationResult{{{QStringLiteral("__image"), QVariant::fromValue(restored)},
                                        {QStringLiteral("imageDataUrl"), preview},
                                        {QStringLiteral("width"), restored.width()},
                                        {QStringLiteral("height"), restored.height()}},
                                       {},
                                       {},
                                       false};
            },
            [this](QVariantMap &result) { image_state_->applyWorkerResult(result); });
        return;
    }

    if (operation != QStringLiteral("imageEdit") || !image_state_->hasCurrentImage()) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("Open an image before editing it."));
        operations_.remove(requestId);
        return;
    }
    const QImage source = image_state_->currentImage();
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
        },
        [this](QVariantMap &result) { image_state_->applyWorkerResult(result); });
}
