#include "image_resize.h"

#include <QTextStream>
#include "core/exception/invalid_argument_exception.h"
#include "core/exception/invalid_state_exception.h"
#include <cmath>

ImageResizeInterface::ImageResizeInterface(QObject *parent)
    : Tool(Tool::ID::IMAGE_RESIZE, "image-resize", parent)
{}

const QString ImageResize::invalidImageSize = "invalid image size";
const QString ImageResize::invalidScale = "invalid scale";
const QString ImageResize::widthOverwritten = "width is already set";
const QString ImageResize::heightOverwritten = "height is already set";

bool ImageResize::loadImpl(const QString &path)
{
    const bool result = ImageIO::load(path);
    _current = original();

    return result && !_current.isNull();
}

bool ImageResize::updateImpl()
{
    _current = std::move(
        original().scaled(computedSize(), Qt::IgnoreAspectRatio, transformationMode()));

    return !_current.isNull();
}

void ImageResize::resetImpl()
{
    width.type = ResizeHints::Type::DEFAULT;
    height.type = ResizeHints::Type::DEFAULT;

    _current = original();
}

void ImageResize::setScale(double sx, double sy)
{
    setScaleX(sx);
    setScaleY(sy);
}

void ImageResize::setScaleX(double sx)
{
    if (sx < 0)
        throw InvalidArgumentException(sx, invalidScale);

    if (width.type != ResizeHints::Type::DEFAULT)
        qWarning() << "horizontal scale is already set";

    width.type = ResizeHints::Type::SCALE;
    width.scale = sx;

    setOutdated();
}

void ImageResize::setScaleY(double sy)
{
    if (sy < 0)
        throw InvalidArgumentException(sy, invalidScale);

    if (height.type != ResizeHints::Type::DEFAULT)
        qWarning() << "vertical scale is already set";

    height.type = ResizeHints::Type::SCALE;
    width.scale = sy;

    setOutdated();
}

void ImageResize::setSize(const QSize &size)
{
    if (width.type != ResizeHints::Type::DEFAULT)
        qWarning() << widthOverwritten;
    if (height.type != ResizeHints::Type::DEFAULT)
        qWarning() << heightOverwritten;

    width.type = ResizeHints::Type::SIZE;
    width.size = size.width();
    height.type = ResizeHints::Type::SIZE;
    height.size = size.height();

    setOutdated();
}

void ImageResize::setWidth(unsigned int w, bool keepAspectRatio)
{
    if (keepAspectRatio)
        setSize(QSize(w, (int) std::round(w / aspectRatio())));
    else {
        if (width.type != ResizeHints::Type::DEFAULT)
            qWarning() << widthOverwritten;

        width.type = ResizeHints::Type::SIZE;
        width.size = w;

        setOutdated();
    }
}

void ImageResize::setHeight(unsigned int h, bool keepAspectRatio)
{
    if (keepAspectRatio)
        setSize(QSize((int) std::round(h * aspectRatio()), h));
    else {
        if (height.type != ResizeHints::Type::DEFAULT)
            qWarning() << heightOverwritten;

        height.type = ResizeHints::Type::SIZE;
        height.size = h;

        setOutdated();
    }
}

unsigned int ImageResize::computedSizeInternal(unsigned int originalSize,
                                               const ResizeHints &hints) const
{
    switch (hints.type) {
    case ResizeHints::Type::SCALE:
        if (originalSize != 0)
            return hints.scale * originalSize;
        else
            throw InvalidArgumentException(originalSize, invalidImageSize);
    case ResizeHints::Type::SIZE:
        return hints.size;
    default:
        return originalSize;
    }
}

double ImageResize::computedScaleInternal(unsigned int originalSize, const ResizeHints &hints) const
{
    switch (hints.type) {
    case ResizeHints::Type::SCALE:
        return hints.scale;
    case ResizeHints::Type::SIZE:
        if (originalSize != 0)
            return (double) hints.size / originalSize;
        else
            throw InvalidArgumentException(originalSize, invalidImageSize);
    default:
        return 1;
    }
}

QSize ImageResize::computedSize() const
{
    const auto size = original().size();
    return QSize(computedSizeInternal(size.width(), width),
                 computedSizeInternal(size.height(), height));
}

double ImageResize::computedScaleX() const
{
    const auto size = original().size();
    if (size.isEmpty())
        throw InvalidStateException(QString("image size: (%1, %2)")
                                        .arg(size.width())
                                        .arg(size.height()),
                                    invalidImageSize);

    return computedScaleInternal(size.width(), width);
}

double ImageResize::computedScaleY() const
{
    const auto size = original().size();
    if (size.isEmpty())
        throw InvalidStateException(QString("image size: (%1, %2)")
                                        .arg(size.width())
                                        .arg(size.height()),
                                    invalidImageSize);

    return computedScaleInternal(size.height(), height);
}

double ImageResize::aspectRatio() const
{
    const auto size = original().size();
    if (size.isEmpty())
        throw InvalidStateException(QString("image size: (%1, %2)")
                                        .arg(size.width())
                                        .arg(size.height()),
                                    invalidImageSize);

    return (double) size.width() / size.height();
}

void ImageResize::setSmoothTransformationEnabled(bool value)
{
    if (value != smoothTransformationEnabled) {
        smoothTransformationEnabled = value;
        setOutdated();
    }
}
