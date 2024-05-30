#include "image_division.h"

#include "core/exception/invalid_argument_exception.h"

ImageDivisionInterface::ImageDivisionInterface(QObject *parent)
    : Tool(Tool::ID::IMAGE_DIVISION, "image-division", parent)
{}

ImageDivision::ImageDivision(QObject *parent)
    : ImageDivisionInterface(parent)
{}

const QString ImageDivision::invalidDivisionNumber = "invalid division number";
const QString ImageDivision::invalidCellSize = "invalid cell size";
const QString ImageDivision::invalidImageSize = "invalid image";

bool ImageDivision::saveImpl(bool (*saveFunc)(const QString &, const QImage &, const char *, int),
                          const QString &path,
                          const char *format,
                          int quality) const
{
    if (current().isNull() || fileInfo().path().isEmpty())
        return false;

    const unsigned int yMax = numberOfVerticalDivision();
    const unsigned int xMax = numberOfHorizontalDivision();
    if (yMax == 0 || xMax == 0)
        return false;

    QFileInfo info(path);
    if (info.exists() && info.isDir()) {
        const QSizeF cellSize = computedCellSize();
        const QDir dir(path);

        bool result = true;

        for (unsigned int y = 0; y < yMax; y++)
            for (unsigned int x = 0; x < xMax; x++) {
                const QRect rect(QPoint(x * cellSize.width(), y * cellSize.height()),
                                 cellSize.toSize());
                const QString filename = saveFilename(dir, x, y);

                result = result && saveFunc(filename, current().copy(rect), format, quality);
            }

        return result;
    }

    return false;
}

QString ImageDivision::saveFilename(const QDir &location, unsigned int x, unsigned int y) const
{
    const QString base = fileInfo().baseName();
    const QString extension = fileInfo().suffix();
    return location.filePath(base % QString("_%1_%2.").arg(x).arg(y) % extension);
}

bool ImageDivision::loadImpl(const QString &path)
{
    const bool result = ImageIO::load(path);
    _current = original();

    return result && !_current.isNull();
}

void ImageDivision::resetImpl()
{
    horizontal = DivisionHints();
    vertical = DivisionHints();
}

void ImageDivision::setHorizontalDivision(unsigned int n)
{
    horizontal.isSpecifiedWithSize = false;
    horizontal.value = n;

    setOutdated();
}

void ImageDivision::setVerticalDivision(unsigned int m)
{
    vertical.isSpecifiedWithSize = false;
    vertical.value = m;

    setOutdated();
}

void ImageDivision::setCellWidth(unsigned int width)
{
    horizontal.isSpecifiedWithSize = true;
    horizontal.value = width;

    setOutdated();
}

void ImageDivision::setCellHeight(unsigned int height)
{
    vertical.isSpecifiedWithSize = true;
    vertical.value = height;

    setOutdated();
}

QSizeF ImageDivision::computedCellSize() const
{
    const QSize size = current().size();
    if (size.isEmpty())
        return QSizeF(0, 0);

    return QSizeF(computedCellSizeInternal(size.width(), horizontal),
                  computedCellSizeInternal(size.height(), vertical));
}

double ImageDivision::computedCellSizeInternal(unsigned int sourceSize, const DivisionHints &hint)
{
    qDebug() << "sourceSize=" << sourceSize << ", hint.isSpecifiedWithSize"
             << hint.isSpecifiedWithSize << ", hint.value=" << hint.value;
    if (sourceSize == 0 || hint.value == 0)
        throw InvalidArgumentException(0, invalidImageSize);

    if (hint.isSpecifiedWithSize)
        return hint.value;
    else {
        if (sourceSize < hint.value)
            throw InvalidArgumentException(hint.value, invalidDivisionNumber);

        return (double) sourceSize / (double) hint.value;
    }
}

unsigned int ImageDivision::numberOfHorizontalDivision() const
{
    if (current().size().isEmpty())
        return 0;
    else
        return numberOfDivisionInternal(current().size().width(), horizontal);
}

unsigned int ImageDivision::numberOfVerticalDivision() const
{
    if (current().size().isEmpty())
        return 0;
    else
        return numberOfDivisionInternal(current().size().height(), vertical);
}

unsigned int ImageDivision::numberOfDivisionInternal(unsigned int sourceSize, const DivisionHints &hint) const
{
    if (sourceSize == 0 || hint.value == 0)
        throw InvalidArgumentException(0, invalidImageSize);

    if (hint.isSpecifiedWithSize) {
        if (sourceSize < hint.value)
            throw InvalidArgumentException(hint.value, invalidDivisionNumber);

        const unsigned int result = sourceSize / hint.value;
        if (discardRemainders || (sourceSize % hint.value == 0))
            return result;
        else
            return result + 1;
    } else
        return hint.value;
}
