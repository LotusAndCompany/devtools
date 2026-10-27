#include "qr_image.h"

#include "qrcodegen.hpp"

namespace {
constexpr int QR_SCALE = 8;
}

QImage QRImage::generate(const QString &content)
{
    const qrcodegen::QrCode qr =
        qrcodegen::QrCode::encodeText(content.toUtf8().constData(), qrcodegen::QrCode::Ecc::LOW);
    const int size = qr.getSize();
    QImage image(size * QR_SCALE, size * QR_SCALE, QImage::Format_RGB32);
    image.fill(Qt::white);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (qr.getModule(x, y)) {
                for (int dy = 0; dy < QR_SCALE; ++dy) {
                    for (int dx = 0; dx < QR_SCALE; ++dx) {
                        image.setPixel((x * QR_SCALE) + dx, (y * QR_SCALE) + dy, qRgb(0, 0, 0));
                    }
                }
            }
        }
    }
    return image;
}

bool QRImage::savePng(const QImage &image, const QString &filePath)
{
    return image.save(filePath, "PNG");
}
