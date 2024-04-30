#include "image_rotation.h"

#include <QRect>

ImageRotationInterface::ImageRotationInterface(QObject *parent)
    : Tool(Tool::ID::IMAGE_ROTATION, "image-rotation", parent)
{}

ImageRotation::ImageRotation(QObject *parent)
    : ImageRotationInterface(parent)
{}

bool ImageRotation::loadImpl(const QString &path)
{
    const bool result = ImageIO::load(path);
    _current = original();

    return result && !_current.isNull();
}

void ImageRotation::resetImpl()
{
    transform.reset();
}

bool ImageRotation::updateImpl()
{
    _current = std::move(original().transformed(transform, transformationMode()));

    return !_current.isNull();
}

void ImageRotation::rotateDegrees(double deg)
{
    const auto size = current().size();
    const double tx = size.width() / 2.0, ty = size.height() / 2.0;

    transform.translate(-tx, -ty).rotate(deg);
    const QRectF rect(QPoint(-tx, -ty), size);
    const QRectF transformedRect = transform.mapRect(rect);
    const QSizeF newSize = transformedRect.size();
    transform.translate(newSize.width() / 2.0, newSize.height() / 2.0);

    setOutdated();
}

void ImageRotation::flipHorizontally()
{
    transform *= QTransform(-1.0, 0.0, 0.0, 1.0, current().size().width(), 0.0);

    setOutdated();
}

void ImageRotation::flipVertically()
{
    transform *= QTransform(1.0, 0.0, 0.0, -1.0, 0.0, current().size().height());

    setOutdated();
}

void ImageRotation::setSmoothTransformationEnabled(bool value)
{
    if (value != smoothTransformationEnabled) {
        smoothTransformationEnabled = value;
        setOutdated();
    }
}
