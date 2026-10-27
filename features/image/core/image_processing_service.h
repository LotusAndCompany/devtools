#ifndef IMAGE_PROCESSING_SERVICE_H
#define IMAGE_PROCESSING_SERVICE_H

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QString>

#include <cstdint>

enum class ImageFlipDirection : uint8_t { Horizontal, Vertical };
enum class ImageSplitMode : uint8_t { DivisionCount, CellSize };

struct ImageSplitOptions
{
    ImageSplitMode mode{ImageSplitMode::DivisionCount};
    int horizontalDivisions{1};
    int verticalDivisions{1};
    int cellWidth{1};
    int cellHeight{1};
    bool ignoreRemainders{false};
};

class ImageProcessingService
{
public:
    static QImage load(const QString &path);
    static bool save(const QImage &image, const QString &path);
    static QImage resizeBySize(const QImage &image, int width, int height, bool keepAspectRatio,
                               bool smoothScaling);
    static QImage resizeByScale(const QImage &image, double horizontalPercent,
                                double verticalPercent, bool keepAspectRatio, bool smoothScaling);
    static QImage rotateLeft(const QImage &image);
    static QImage rotateRight(const QImage &image);
    static QImage flip(const QImage &image, ImageFlipDirection direction);
    static uint8_t alphaForTransparency(double transparency);
    static QImage makeTransparent(const QImage &image, const QPoint &point,
                                  const QColor &selectedColor, QColor::Spec colorSpec,
                                  double tolerance, uint8_t opacity, bool contiguousOnly);
    static bool saveDividedImages(const QImage &image, const QString &originalPath,
                                  const QString &folderPath, const ImageSplitOptions &options);
};

#endif // IMAGE_PROCESSING_SERVICE_H
