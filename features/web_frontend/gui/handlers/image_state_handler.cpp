#include "image_state_handler.h"

#include "features/image/core/image_processing_service.h"

#include <QMetaType>
#include <QVariant>

#include <utility>

bool ImageStateHandler::queueDroppedImage(const QString &path)
{
    if (!ImageProcessingService::isReadableImageFile(path)) {
        return false;
    }
    dropped_image_path_ = path;
    return true;
}

QString ImageStateHandler::takeDroppedImagePath()
{
    return std::exchange(dropped_image_path_, QString());
}

bool ImageStateHandler::hasDroppedImage() const
{
    return !dropped_image_path_.isEmpty();
}

bool ImageStateHandler::hasCurrentImage() const
{
    return !current_image_.isNull();
}

const QImage &ImageStateHandler::currentImage() const
{
    return current_image_;
}

const QImage &ImageStateHandler::originalImage() const
{
    return original_image_;
}

const QString &ImageStateHandler::currentImagePath() const
{
    return current_image_path_;
}

void ImageStateHandler::applyWorkerResult(QVariantMap &result)
{
    const QVariant imageValue = result.take(QStringLiteral("__image"));
    if (imageValue.metaType().id() == QMetaType::QImage) {
        current_image_ = imageValue.value<QImage>();
    }
    const QVariant originalImageValue = result.take(QStringLiteral("__originalImage"));
    if (originalImageValue.metaType().id() == QMetaType::QImage) {
        original_image_ = originalImageValue.value<QImage>();
    }
    const QString imagePath = result.take(QStringLiteral("__imagePath")).toString();
    if (!imagePath.isEmpty()) {
        current_image_path_ = imagePath;
    }
}
