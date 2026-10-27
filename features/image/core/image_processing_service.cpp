#include "image_processing_service.h"

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QQueue>
#include <QTransform>
#include <QVector3D>

#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr double M_TAU = 2.0 * M_PI;

struct DivisionPlan
{
    int xCount{0};
    int yCount{0};
    double stepW{0.0};
    double stepH{0.0};
};

int countByCellSize(int source, int cell, bool ignoreRemainders)
{
    if (source <= 0 || cell <= 0) {
        return 0;
    }
    const int base = source / cell;
    if (ignoreRemainders || (source % cell == 0)) {
        return base;
    }
    return base + 1;
}

DivisionPlan buildDivisionPlan(const ImageSplitOptions &options, int sourceW, int sourceH)
{
    DivisionPlan plan;
    if (options.mode == ImageSplitMode::DivisionCount) {
        plan.xCount = std::max(1, options.horizontalDivisions);
        plan.yCount = std::max(1, options.verticalDivisions);
        plan.stepW = static_cast<double>(sourceW) / plan.xCount;
        plan.stepH = static_cast<double>(sourceH) / plan.yCount;
    } else {
        const int cellW = std::max(1, options.cellWidth);
        const int cellH = std::max(1, options.cellHeight);
        plan.xCount = countByCellSize(sourceW, cellW, options.ignoreRemainders);
        plan.yCount = countByCellSize(sourceH, cellH, options.ignoreRemainders);
        plan.stepW = cellW;
        plan.stepH = cellH;
    }
    return plan;
}

QRect divisionRect(const ImageSplitOptions &options, const DivisionPlan &plan, int x, int y,
                   int sourceW, int sourceH)
{
    int left = static_cast<int>(std::floor(x * plan.stepW));
    int top = static_cast<int>(std::floor(y * plan.stepH));
    int right = static_cast<int>(std::floor((x + 1) * plan.stepW));
    int bottom = static_cast<int>(std::floor((y + 1) * plan.stepH));

    if (!options.ignoreRemainders) {
        if (x == plan.xCount - 1) {
            right = sourceW;
        }
        if (y == plan.yCount - 1) {
            bottom = sourceH;
        }
    }

    left = std::clamp(left, 0, sourceW);
    top = std::clamp(top, 0, sourceH);
    right = std::clamp(right, 0, sourceW);
    bottom = std::clamp(bottom, 0, sourceH);
    if (right <= left || bottom <= top) {
        return {};
    }
    return {left, top, right - left, bottom - top};
}

double colorDiffSquaredRgb(const QColor &a, const QColor &b)
{
    const double dr = a.redF() - b.redF();
    const double dg = a.greenF() - b.greenF();
    const double db = a.blueF() - b.blueF();
    return (dr * dr) + (dg * dg) + (db * db);
}

QVector3D hsvVector(const QColor &color)
{
    if (color.hsvHueF() < 0) {
        return {0, 0, static_cast<float>(color.valueF())};
    }
    return {static_cast<float>(color.hsvSaturationF() * std::cos(M_TAU * color.hsvHueF())),
            static_cast<float>(color.hsvSaturationF() * std::sin(M_TAU * color.hsvHueF())),
            static_cast<float>(color.valueF())};
}

QVector3D hslVector(const QColor &color)
{
    if (color.hslHueF() < 0) {
        return {0, 0, static_cast<float>(color.lightnessF())};
    }
    return {static_cast<float>(color.hslSaturationF() * std::cos(M_TAU * color.hslHueF())),
            static_cast<float>(color.hslSaturationF() * std::sin(M_TAU * color.hslHueF())),
            static_cast<float>(color.lightnessF())};
}

double colorDiffSquaredHsv(const QColor &a, const QColor &b)
{
    return (hsvVector(b) - hsvVector(a)).lengthSquared();
}

double colorDiffSquaredHsl(const QColor &a, const QColor &b)
{
    return (hslVector(b) - hslVector(a)).lengthSquared();
}

double maxColorDiffSquared(QColor::Spec spec)
{
    switch (spec) {
    case QColor::Spec::Rgb:
        return 3.0;
    case QColor::Spec::Hsv:
    case QColor::Spec::Hsl:
        return 4.0;
    default:
        return 3.0;
    }
}

bool isTransparentTarget(const QColor &source, const QColor &target, QColor::Spec spec,
                         double tolerance)
{
    const QColor a = source.convertTo(spec);
    const QColor b = target.convertTo(spec);
    double difference = 0.0;
    switch (spec) {
    case QColor::Spec::Rgb:
        difference = colorDiffSquaredRgb(a, b);
        break;
    case QColor::Spec::Hsv:
        difference = colorDiffSquaredHsv(a, b);
        break;
    case QColor::Spec::Hsl:
        difference = colorDiffSquaredHsl(a, b);
        break;
    default:
        difference = colorDiffSquaredRgb(a, b);
        break;
    }
    return difference <= (maxColorDiffSquared(spec) * tolerance * tolerance);
}

void setAlpha(QImage &image, const QPoint &point, uint8_t opacity)
{
    QColor color = image.pixelColor(point);
    color.setAlpha(opacity);
    image.setPixelColor(point, color);
}

QImage makeColorTransparent(const QImage &source, const QColor &targetColor, QColor::Spec spec,
                            double tolerance, uint8_t opacity)
{
    QImage image = source;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            if (isTransparentTarget(color, targetColor, spec, tolerance)) {
                color.setAlpha(opacity);
                image.setPixelColor(x, y, color);
            }
        }
    }
    return image;
}

QImage makeContiguousTransparent(const QImage &source, const QPoint &start, QColor::Spec spec,
                                 double tolerance, uint8_t opacity)
{
    QImage image = source;
    if (!image.valid(start)) {
        return image;
    }

    const QColor targetColor = image.pixelColor(start);
    QVector<bool> visited(static_cast<qsizetype>(image.width()) * image.height(), false);
    const auto index = [&image](int x, int y) {
        return (static_cast<qsizetype>(y) * static_cast<qsizetype>(image.width())) + x;
    };

    QQueue<QPoint> queue;
    queue.enqueue(start);
    visited[index(start.x(), start.y())] = true;
    while (!queue.isEmpty()) {
        const QPoint point = queue.dequeue();
        setAlpha(image, point, opacity);
        const std::array<QPoint, 4> neighbors = {
            QPoint(point.x() + 1, point.y()),
            QPoint(point.x() - 1, point.y()),
            QPoint(point.x(), point.y() + 1),
            QPoint(point.x(), point.y() - 1),
        };
        for (const QPoint &neighbor : neighbors) {
            if (!image.valid(neighbor)) {
                continue;
            }
            const qsizetype neighborIndex = index(neighbor.x(), neighbor.y());
            if (visited[neighborIndex]) {
                continue;
            }
            visited[neighborIndex] = true;
            if (isTransparentTarget(image.pixelColor(neighbor), targetColor, spec, tolerance)) {
                queue.enqueue(neighbor);
            }
        }
    }
    return image;
}
} // namespace

bool ImageProcessingService::isReadableImageFile(const QString &path)
{
    return QFileInfo(path).isFile() && !QImageReader::imageFormat(path).isEmpty();
}

QImage ImageProcessingService::load(const QString &path)
{
    return QImage(path).convertToFormat(QImage::Format_RGBA8888);
}

bool ImageProcessingService::save(const QImage &image, const QString &path)
{
    return image.save(path);
}

QImage ImageProcessingService::resizeBySize(const QImage &image, int width, int height,
                                            bool keepAspectRatio, bool smoothScaling)
{
    if (image.isNull() || width <= 0 || height <= 0) {
        return image;
    }
    if (keepAspectRatio) {
        const double ratio = static_cast<double>(image.width()) / image.height();
        height = static_cast<int>(std::round(width / ratio));
    }
    if (height <= 0) {
        return image;
    }
    const Qt::TransformationMode mode =
        smoothScaling ? Qt::SmoothTransformation : Qt::FastTransformation;
    return image.scaled(QSize(width, height), Qt::IgnoreAspectRatio, mode);
}

QImage ImageProcessingService::resizeByScale(const QImage &image, double horizontalPercent,
                                             double verticalPercent, bool keepAspectRatio,
                                             bool smoothScaling)
{
    if (image.isNull()) {
        return image;
    }
    const double horizontalScale = horizontalPercent / 100.0;
    const double verticalScale = keepAspectRatio ? horizontalScale : verticalPercent / 100.0;
    const int width = std::max(1, static_cast<int>(std::round(image.width() * horizontalScale)));
    const int height = std::max(1, static_cast<int>(std::round(image.height() * verticalScale)));
    return resizeBySize(image, width, height, false, smoothScaling);
}

QImage ImageProcessingService::rotateLeft(const QImage &image)
{
    QTransform transform;
    transform.rotate(-90);
    return image.transformed(transform, Qt::SmoothTransformation);
}

QImage ImageProcessingService::rotateRight(const QImage &image)
{
    QTransform transform;
    transform.rotate(90);
    return image.transformed(transform, Qt::SmoothTransformation);
}

QImage ImageProcessingService::flip(const QImage &image, ImageFlipDirection direction)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    return image.flipped(direction == ImageFlipDirection::Horizontal ? Qt::Horizontal
                                                                     : Qt::Vertical);
#else
    return image.mirrored(direction == ImageFlipDirection::Horizontal,
                          direction == ImageFlipDirection::Vertical);
#endif
}

uint8_t ImageProcessingService::alphaForTransparency(double transparency)
{
    const double clamped = std::clamp(transparency, 0.0, 1.0);
    return static_cast<uint8_t>(255 * (1.0 - clamped));
}

QImage ImageProcessingService::makeTransparent(const QImage &image, const QPoint &point,
                                               const QColor &selectedColor, QColor::Spec colorSpec,
                                               double tolerance, uint8_t opacity,
                                               bool contiguousOnly)
{
    if (contiguousOnly) {
        return makeContiguousTransparent(image, point, colorSpec, tolerance, opacity);
    }
    return makeColorTransparent(image, selectedColor, colorSpec, tolerance, opacity);
}

bool ImageProcessingService::saveDividedImages(const QImage &image, const QString &originalPath,
                                               const QString &folderPath,
                                               const ImageSplitOptions &options)
{
    if (image.isNull() || folderPath.isEmpty() || image.width() <= 0 || image.height() <= 0) {
        return false;
    }

    const int sourceW = image.width();
    const int sourceH = image.height();
    const DivisionPlan plan = buildDivisionPlan(options, sourceW, sourceH);
    if (plan.xCount <= 0 || plan.yCount <= 0) {
        return false;
    }

    const QFileInfo info(originalPath);
    const QString baseName = info.baseName().isEmpty() ? QStringLiteral("image") : info.baseName();
    const QString suffix = info.suffix().isEmpty() ? QStringLiteral("png") : info.suffix();
    const QDir directory(folderPath);
    bool result = true;
    for (int y = 0; y < plan.yCount; ++y) {
        for (int x = 0; x < plan.xCount; ++x) {
            const QRect rect = divisionRect(options, plan, x, y, sourceW, sourceH);
            if (rect.isEmpty()) {
                continue;
            }
            const QString path =
                directory.filePath(baseName + QStringLiteral("_%1_%2.").arg(x).arg(y) + suffix);
            result = result && image.copy(rect).save(path);
        }
    }
    return result;
}
