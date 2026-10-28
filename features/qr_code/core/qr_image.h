#ifndef QR_IMAGE_H
#define QR_IMAGE_H

#include <QImage>
#include <QString>

class QRImage
{
public:
    static QImage generate(const QString &content);
    static bool savePng(const QImage &image, const QString &filePath);
};

#endif // QR_IMAGE_H
