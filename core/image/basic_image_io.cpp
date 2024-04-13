#include "basic_image_io.h"

bool ImageIO::load(const QString &path)
{
    _fileInfo = QFileInfo(path);
    _original = std::move(QImage(path));
    return !_original.isNull();
}
