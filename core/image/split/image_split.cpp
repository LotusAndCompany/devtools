#include "image_split.h"

#include "core/exception/invalid_argument_exception.h"

ImageSplitInterface::ImageSplitInterface(QObject *parent)
    : Tool(Tool::ID::IMAGE_SPLIT, "image-split", parent)
{}

ImageSplit::ImageSplit(QObject *parent)
    : ImageSplitInterface(parent)
{}

const QString ImageSplit::invalidSplitNumber = "invalid split number";
const QString ImageSplit::invalidCellSize = "invalid cell size";

bool ImageSplit::save(const QString &path, const char *format, int quality) const
{
    QFileInfo info(path);
    if (info.exists() && info.isDir()) {
    }

    return false;
}

bool ImageSplit::overwriteSave(const QString &path, const char *format, int quality) const
{
    QFileInfo info(path);
    if (info.exists() && info.isDir()) {
    }

    return false;
}

QStringList ImageSplit::outputFilenames() const
{
    const QString base = fileInfo().baseName();
    QStringList result;
    if (!base.isEmpty()) {
    }

    return result;
}

bool ImageSplit::loadImpl(const QString &path)
{
    const bool result = ImageIO::load(path);
    _current = original();

    return result && !_current.isNull();
}

void ImageSplit::resetImpl()
{
    horizontal = SplitHints();
    vertical = SplitHints();
}

void ImageSplit::setHorizontalSplit(unsigned int n)
{
    if (n == 0 || current().size().width() < n)
        throw InvalidArgumentException(n, invalidSplitNumber);

    horizontal.isSpecifiedWithSize = false;
    horizontal.value = n;

    setOutdated();
}

void ImageSplit::setVerticalSplit(unsigned int m)
{
    if (m == 0 || current().size().height() < m)
        throw InvalidArgumentException(m, invalidSplitNumber);

    vertical.isSpecifiedWithSize = false;
    vertical.value = m;

    setOutdated();
}

void ImageSplit::setCellWidth(unsigned int width)
{
    if (width == 0 || current().size().width() < width)
        throw InvalidArgumentException(width, invalidCellSize);

    horizontal.isSpecifiedWithSize = true;
    horizontal.value = width;

    setOutdated();
}

void ImageSplit::setCellHeight(unsigned int height)
{
    if (height == 0 || current().size().height() < height)
        throw InvalidArgumentException(height, invalidCellSize);

    vertical.isSpecifiedWithSize = true;
    vertical.value = height;

    setOutdated();
}

QSizeF ImageSplit::computedCellSize() const {}
