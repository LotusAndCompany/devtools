#ifndef IMAGE_STATE_HANDLER_H
#define IMAGE_STATE_HANDLER_H

#include <QImage>
#include <QString>
#include <QVariantMap>

class ImageStateHandler
{
public:
    [[nodiscard]] bool queueDroppedImage(const QString &path);
    [[nodiscard]] QString takeDroppedImagePath();
    [[nodiscard]] bool hasDroppedImage() const;
    [[nodiscard]] bool hasCurrentImage() const;
    [[nodiscard]] const QImage &currentImage() const;
    [[nodiscard]] const QImage &originalImage() const;
    [[nodiscard]] const QString &currentImagePath() const;
    void applyWorkerResult(QVariantMap &result);

private:
    QImage original_image_;
    QImage current_image_;
    QString current_image_path_;
    QString dropped_image_path_;
};

#endif // IMAGE_STATE_HANDLER_H
