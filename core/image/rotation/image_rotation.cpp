#include "image_rotation.h"

#include <QRect>

ImageRotationInterface::ImageRotationInterface(QObject *parent)
    : Tool(Tool::ID::IMAGE_ROTATION, "image-rotation", parent)
{}

ImageRotation::ImageRotation(QObject *parent) : ImageRotationInterface(parent) {}

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
    _current = original().transformed(transform, transformationMode());

    return !_current.isNull();
}

void ImageRotation::rotateDegrees(double deg)
{
    // NOTE: 各座標ではなく座標系の方を回転させるらしい
    transform.rotate(-deg);

    setOutdated();
}

void ImageRotation::flipHorizontal()
{
    transform *= QTransform(-1.0, 0.0, 0.0, 1.0, 0.0, 0.0);

    setOutdated();
}

void ImageRotation::flipVertical()
{
    transform *= QTransform(1.0, 0.0, 0.0, -1.0, 0.0, 0.0);

    setOutdated();
}

void ImageRotation::setSmoothTransformationEnabled(bool value)
{
    if (value != smoothTransformationEnabled) {
        smoothTransformationEnabled = value;
        setOutdated();
    }
}
