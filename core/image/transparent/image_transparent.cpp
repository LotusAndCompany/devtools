#include "image_transparent.h"

#include <QVector3D>

const QString ImageTransparent::expectedColorSpec = QString("available colorSpec values=%1");
const QString ImageTransparent::expectedImageFormat
    = QString("image format=%1(QImage::Format_RGBA8888)").arg(QImage::Format_RGBA8888);
const QString ImageTransparent::pixelOutOfImage = "pixel must be inside the image (%1 x %2)";

ImageTransparentInterface::ImageTransparentInterface(QObject *parent)
    : Tool(Tool::ID::IMAGE_TRANSPARENT, "image-transparent", parent)
{}

ImageTransparent::ImageTransparent(QObject *parent)
    : ImageTransparentInterface(parent)
{}

bool ImageTransparent::loadImpl(const QString &path)
{
    const bool result = ImageIO::load(path);
    fetchOriginal();

    return result && !_current.isNull();
}

void ImageTransparent::resetImpl()
{
    fetchOriginal();
}

void ImageTransparent::fetchOriginal()
{
    _current = std::move(original().convertToFormat(QImage::Format_RGBA8888));
}

void ImageTransparent::addTransparentColor(const QColor &targetColor)
{
    if (current().isNull())
        return;

    if (targetColor.spec() != colorSpec) {
        // 色空間を揃える
        addTransparentColor(targetColor.convertTo(colorSpec));
        return;
    }

    // NOTE: RGBA8888に変換しているので1pixel=4bytesの想定
    validateImageFormat(current().format());
    const int bytesPerPixel = current().depth() / 8;

    // NOTE: 色の形式はRGB, HSV, HSLを想定
    color_comp_function_type const comparison = colorComparisonFunction(colorSpec);

    const double toleranceSquaredx3 = maxColorDiffSquared(colorSpec) * tolerance * tolerance;
    for (unsigned int y = 0; y < current().height(); y++) {
        // NOTE: [公式ドキュメント](https://doc.qt.io/qt-6/qimage.html#scanLine)では
        //       reinterpret_cast<QRgb*>を使っているが、これが使えるかどうかはフォーマット次第のはず
        uchar *const line = _current.scanLine(y);
        for (unsigned int x = 0; x < current().width(); x++) {
            uchar *const pixel = line + bytesPerPixel * x;
            const uchar r = pixel[0];
            const uchar g = pixel[1];
            const uchar b = pixel[2];

            if (comparison(QColor(r, g, b), targetColor) <= toleranceSquaredx3)
                pixel[3] = opacity;
        }
    }

    // NOTE: _currentは更新済みだが、形式的に設定する
    setOutdated();
}

void ImageTransparent::addTransparentPixel(const QPoint &start)
{
    if (current().isNull())
        return;

    if (!current().valid(start))
        throw InvalidArgumentException<QString>(
            QString("start=(%1, %2)").arg(start.x()).arg(start.y()),
            pixelOutOfImage.arg(current().width()).arg(current().height()));

    // 色空間を揃える
    const QColor targetColor = QColor(current().pixelColor(start)).convertTo(colorSpec);

    // NOTE: RGBA8888に変換しているので1pixel=4bytesの想定
    validateImageFormat(current().format());
    const int bytesPerPixel = current().depth() / 8;

    // NOTE: 色の形式はRGB, HSV, HSLを想定
    color_comp_function_type const comparison = colorComparisonFunction(colorSpec);

    const double toleranceSquaredx3 = maxColorDiffSquared(colorSpec) * tolerance * tolerance;
    // labelsとlookupを設定する。連続領域に同じラベルを設定する。
    QList<int> labels(current().width() * current().height(), 0);
    QMap<int, int> lookup;
    uchar *const data = _current.bits();
    int nextLabel = 0;

    const auto &pixelIndex = [=](int x, int y) {
        if (x < 0 || current().width() <= x)
            throw InvalidArgumentException<int>(x,
                                                QString("x value must be in [0, %1)")
                                                    .arg(current().width()));
        else if (y < 0 || current().height() <= y)
            throw InvalidArgumentException<int>(y,
                                                QString("y value must be in [0, %1)")
                                                    .arg(current().height()));

        return current().width() * y + x;
    };

    for (unsigned int y = 0; y < current().height(); y++) {
        for (unsigned int x = 0; x < current().width(); x++) {
            const int currentPixelIndex = pixelIndex(x, y);
            const int dataIndex = bytesPerPixel * currentPixelIndex;

            const uchar r = data[dataIndex];
            const uchar g = data[dataIndex + 1];
            const uchar b = data[dataIndex + 2];

            if (comparison(QColor(r, g, b), targetColor) <= toleranceSquaredx3) {
                int refLabels[] = {
                    0,
                    0,
                };
                // 四近傍法を使う
                if (0 < y)
                    refLabels[0] = labels[pixelIndex(x, y - 1)];
                if (0 < x)
                    refLabels[1] = labels[pixelIndex(x - 1, y)];

                if (refLabels[0] == 0 && refLabels[1] == 0) {
                    nextLabel++;
                    labels[currentPixelIndex] = nextLabel;
                } else {
                    if (0 < refLabels[0] && 0 < refLabels[1]) {
                        // 小さい方のラベルを設定し、大きい方のラベルをlookupに登録する。
                        // lookupTable[置き換え元] = 置き換え先
                        if (refLabels[0] < refLabels[1]) {
                            labels[currentPixelIndex] = refLabels[0];
                            lookup[refLabels[1]] = refLabels[0];
                        } else if (refLabels[1] < refLabels[0]) {
                            labels[currentPixelIndex] = refLabels[1];
                            lookup[refLabels[0]] = refLabels[1];
                        } else {
                            // 0<refLabels[0]=refLabels[1]
                            labels[currentPixelIndex] = refLabels[0];
                        }
                    } else if (0 < refLabels[0] && refLabels[1] == 0) {
                        labels[currentPixelIndex] = refLabels[0];
                    } else if (refLabels[0] == 0 && 0 < refLabels[1]) {
                        labels[currentPixelIndex] = refLabels[1];
                    }
                }
            }
        }
    }

    // lookupをlabelsに適用する。lookupは空になる想定。
    while (lookup.size()) {
        const auto &it = std::prev(lookup.cend());
        const int from = it.key();
        const int to = it.value();

        for (int &label : labels) {
            if (label == from)
                label = to;
        }

        lookup.erase(it);
    }

    // labelsに基づき透明化する
    const int targetLabel = labels[pixelIndex(start.x(), start.y())];
    for (unsigned int y = 0; y < current().height(); y++) {
        for (unsigned int x = 0; x < current().width(); x++) {
            const int currentPixelIndex = pixelIndex(x, y);
            if (labels[currentPixelIndex] == targetLabel)
                data[bytesPerPixel * currentPixelIndex + 3] = opacity;
        }
    }

    // NOTE: _currentは更新済みだが、形式的に設定する
    setOutdated();
}

double ImageTransparent::colorDiffSquaredRgb(const QColor &a, const QColor &b)
{
    const double diffR = a.redF() - b.redF();
    const double diffG = a.greenF() - b.greenF();
    const double diffB = a.blueF() - b.blueF();

    return diffR * diffR + diffG * diffG + diffB * diffB;
}

constexpr const double M_TAU = 2.0 * M_PI;

double ImageTransparent::colorDiffSquaredHsv(const QColor &a, const QColor &b)
{
    QVector3D va;
    if (a.hsvHueF() < 0)
        va = QVector3D(0, 0, a.valueF());
    else
        va = QVector3D(a.hsvSaturationF() * cos(M_TAU * a.hsvHueF()),
                       a.hsvSaturationF() * sin(M_TAU * a.hsvHueF()),
                       a.valueF());
    QVector3D vb;
    if (b.hsvHueF() < 0)
        vb = QVector3D(0, 0, b.valueF());
    else
        vb = QVector3D(b.hsvSaturationF() * cos(M_TAU * b.hsvHueF()),
                       b.hsvSaturationF() * sin(M_TAU * b.hsvHueF()),
                       b.valueF());

    return (vb - va).lengthSquared();
}

double ImageTransparent::colorDiffSquaredHsl(const QColor &a, const QColor &b)
{
    QVector3D va;
    if (a.hslHueF() < 0)
        va = QVector3D(0, 0, a.lightnessF());
    else
        va = QVector3D(a.hslSaturationF() * cos(M_TAU * a.hslHueF()),
                       a.hslSaturationF() * sin(M_TAU * a.hslHueF()),
                       a.lightnessF());
    QVector3D vb;
    if (b.hslHueF() < 0)
        vb = QVector3D(0, 0, b.lightnessF());
    else
        vb = QVector3D(b.hslSaturationF() * cos(M_TAU * b.hslHueF()),
                       b.hslSaturationF() * sin(M_TAU * b.hslHueF()),
                       b.lightnessF());

    return (vb - va).lengthSquared();
}

ImageTransparent::color_comp_function_type ImageTransparent::colorComparisonFunction(
    QColor::Spec colorSpec)
{
    validateColorSpec(colorSpec);

    switch (colorSpec) {
    case QColor::Rgb:
        return colorDiffSquaredRgb;
    case QColor::Hsv:
        return colorDiffSquaredHsv;
    case QColor::Hsl:
        return colorDiffSquaredHsl;
    default:
        // unreachable
        return nullptr;
    }
}

double ImageTransparent::maxColorDiffSquared(QColor::Spec colorSpec)
{
    validateColorSpec(colorSpec);

    switch (colorSpec) {
    case QColor::Rgb:
        return 3.0;
    case QColor::Hsv:
    case QColor::Hsl:
        return 4.0;
    default:
        // unreachable
        return -1.0;
    }
}

void ImageTransparent::validateColorSpec(QColor::Spec colorSpec) noexcept(false)
{
    switch (colorSpec) {
    case QColor::Rgb:
    case QColor::Hsv:
    case QColor::Hsl:
        return;
    default:
        break;
    }

    const QList<QColor::Spec> avaliableColorSpecList = {
        QColor::Rgb,
        QColor::Hsv,
        QColor::Hsl,
    };
    QStringList strings(avaliableColorSpecList.size());
    for (int i : avaliableColorSpecList)
        strings.append(QString::number(i));
    throw InvalidArgumentException<int>(colorSpec, expectedColorSpec.arg(strings.join(',')));
}
