#include "image_transparent.h"

const QString ImageTransparent::expectedColorSpec = QString("available colorSpec values=%1");
const QString ImageTransparent::expectedImageFormat
    = QString("image format=%1(QImage::Format_RGBA8888)").arg(QImage::Format_RGBA8888);
const QString ImageTransparent::pixelOutOfImage = "pixel must be inside the image (%1 x %2)";

ImageTransparentInterface::ImageTransparentInterface(QObject *parent)
    : Tool(Tool::ID::IMAGE_TRANSPARENT, "image-transparent", parent)
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
    const int depth = current().depth();

    // NOTE: 色の形式はRGB, HSV, HSLを想定
    color_comp_function_type const comparison = colorComparisonFunction(colorSpec);

    const double tolerance2 = tolerance * tolerance;
    for (unsigned int y = 0; y < current().height(); y++) {
        // NOTE: [公式ドキュメント](https://doc.qt.io/qt-6/qimage.html#scanLine)では
        //       reinterpret_cast<QRgb*>を使っているが、これが使えるかどうかはフォーマット次第のはず
        uchar *const line = _current.scanLine(y);
        for (unsigned int x = 0; x < current().width(); x++) {
            uchar *const pixel = line + depth * x;
            const uchar r = pixel[0];
            const uchar g = pixel[1];
            const uchar b = pixel[2];

            if (comparison(QColor(r, g, b), targetColor) <= tolerance2)
                pixel[3] = transparency;
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
        throw InvalidArgumentException<QPoint>(start,
                                               pixelOutOfImage.arg(current().width())
                                                   .arg(current().height()));

    // 色空間を揃える
    const QColor targetColor = QColor(current().pixelColor(start)).convertTo(colorSpec);

    // NOTE: RGBA8888に変換しているので1pixel=4bytesの想定
    validateImageFormat(current().format());
    const int depth = current().depth();

    // NOTE: 色の形式はRGB, HSV, HSLを想定
    color_comp_function_type const comparison = colorComparisonFunction(colorSpec);

    const double tolerance2 = tolerance * tolerance;

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

double ImageTransparent::colorDiffSquaredHsv(const QColor &a, const QColor &b)
{
    double diffH = a.hsvHueF() - b.hsvHueF();
    if (180 < diffH)
        diffH -= 360;
    else if (diffH < -180)
        diffH += 360;
    diffH /= 360.0;

    const double diffS = a.hsvSaturationF() - b.hsvSaturationF();
    const double diffV = a.valueF() - b.valueF();

    return diffH * diffH + diffS * diffS + diffV * diffV;
}

double ImageTransparent::colorDiffSquaredHsl(const QColor &a, const QColor &b)
{
    double diffH = a.hslHueF() - b.hslHueF();
    if (180 < diffH)
        diffH -= 360;
    else if (diffH < -180)
        diffH += 360;
    diffH /= 360.0;

    const double diffS = a.hslSaturationF() - b.hslSaturationF();
    const double diffL = a.lightnessF() - b.lightnessF();

    return diffH * diffH + diffS * diffS + diffL * diffL;
}

ImageTransparent::color_comp_function_type ImageTransparent::colorComparisonFunction(
    QColor::Spec colorSpec)
{
    switch (colorSpec) {
    case QColor::Rgb:
        return colorDiffSquaredRgb;
    case QColor::Hsv:
        return colorDiffSquaredHsv;
    case QColor::Hsl:
        return colorDiffSquaredHsl;
    default:
        // unexpected
        break;
    }

    const QList<int> avaliableColorSpecList = {
        QColor::Rgb,
        QColor::Hsv,
        QColor::Hsl,
    };
    QStringList strings(avaliableColorSpecList.size());
    for (int i : avaliableColorSpecList)
        strings.append(QString::number(i));
    throw InvalidArgumentException<int>(colorSpec, expectedColorSpec.arg(strings.join(',')));
}
