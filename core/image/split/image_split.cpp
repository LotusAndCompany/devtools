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
const QString ImageSplit::invalidImageSize = "invalid image";

bool ImageSplit::saveImpl(bool (*saveFunc)(const QString &, const QImage &, const char *, int),
                          const QString &path,
                          const char *format,
                          int quality) const
{
    if (current().size().isEmpty())
        return false;

    QFileInfo info(path);
    if (info.exists() && info.isDir()) {
        const QString base = fileInfo().baseName();

        if (!base.isEmpty()) {
            const QSize size = current().size();
            const unsigned int yMax = numberOfSplit(size.height(), vertical);
            const unsigned int xMax = numberOfSplit(size.width(), horizontal);

            if (yMax == 0 || xMax == 0)
                return false;

            const QString extension = fileInfo().suffix();
            const QSizeF cellSize = computedCellSize();
            const QDir dir(path);

            bool result = true;

            for (unsigned int y = 0; y < yMax; y++)
                for (unsigned int x = 0; x < xMax; x++) {
                    const QRect rect(QPoint(x * cellSize.width(), y * cellSize.height()),
                                     cellSize.toSize());
                    const QString filename = dir.filePath(base % QString("_%3_%4.").arg(x).arg(y)
                                                          % extension);

                    result = result && saveFunc(filename, current().copy(rect), format, quality);
                }

            return result;
        }
    }

    return false;
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

QSizeF ImageSplit::computedCellSize() const
{
    const QSize size = current().size();

    return QSizeF(computedCellSizeInternal(size.width(), horizontal),
                  computedCellSizeInternal(size.height(), vertical));
}

double ImageSplit::computedCellSizeInternal(unsigned int sourceSize, const SplitHints &hint)
{
    if (sourceSize == 0 || hint.value == 0)
        throw InvalidArgumentException(0, invalidImageSize);

    if (hint.isSpecifiedWithSize)
        return hint.value;
    else
        return (double) sourceSize / (double) hint.value;
}

unsigned int ImageSplit::numberOfSplit(unsigned int sourceSize, const SplitHints &hint) const
{
    if (sourceSize == 0 || hint.value == 0)
        throw InvalidArgumentException(0, invalidImageSize);

    if (hint.isSpecifiedWithSize) {
        if (hint.value < sourceSize)
            throw InvalidArgumentException(hint.value, invalidSplitNumber);

        const unsigned int result = sourceSize / hint.value;
        if (discardRemainders || (sourceSize % hint.value == 0))
            return result;
        else
            return result + 1;
    } else
        return hint.value;
}
