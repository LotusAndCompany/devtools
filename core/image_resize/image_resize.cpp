#include "image_resize.h"

#include "core/exception/invalid_argument_exception.h"

ImageResizeInterface::ImageResizeInterface(QObject *parent)
    : Tool(Tool::ID::IMAGE_RESIZE, "image-resize", parent)
{}

const QString ImageResize::invalidScaleReason = "scale must be grater than or equal to 0";
const QString ImageResize::invalidSizeReason
    = "both width and height must be grater than or equal to 0";

ImageResize::ImageResize(QObject *parent)
    : ImageResizeInterface(parent)
{}

bool ImageResize::load(const QString &path)
{
    original = std::move(QPixmap(path));
    pixmap = original;

    if (!original.isNull()) {
        currentFileInfo.setFile(path);
        return true;
    } else {
        currentFileInfo = QFileInfo();
        return false;
    }
}

bool ImageResize::save(const QString &path, const char *format) const
{
    // NOTE: pathが無効ならsaveは失敗するはず
    return pixmap.save(path, format);
}

void ImageResize::setScaleX(double sx)
{
    if (sx < 0)
        throw InvalidArgumentException(sx, invalidScaleReason);

    setWidth(originalSize().width() * sx);
}

void ImageResize::setScaleY(double sy)
{
    if (sy < 0)
        throw InvalidArgumentException(sy, invalidScaleReason);

    setHeight(originalSize().height() * sy);
}

void ImageResize::setWidth(unsigned int w)
{
    if (keepAspectRatio) {
        pixmap = std::move(original.scaledToWidth(w, transformationMode()));
    } else {
        pixmap = std::move(
            original.scaled(w, currentSize().height(), Qt::IgnoreAspectRatio, transformationMode()));
    }
}

void ImageResize::setHeight(unsigned int h)
{
    if (keepAspectRatio) {
        pixmap = std::move(original.scaledToHeight(h, transformationMode()));
    } else {
        pixmap = std::move(
            original.scaled(currentSize().width(), h, Qt::IgnoreAspectRatio, transformationMode()));
    }
}
