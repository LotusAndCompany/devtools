#ifndef IMAGE_EDITOR_CONTEXT_H
#define IMAGE_EDITOR_CONTEXT_H

#include "QtCore/qobject.h"
#include "core/image/basic_image_edit_interface.h"
class ImageEditorContext : public QObject
{
    Q_OBJECT
public:
    explicit ImageEditorContext(BasicImageEditInterface *editor,
                                QObject *parent = nullptr);

    bool loadImage(const QString &path);
    bool saveImage(const QString &path);
    void reset();

    void setWidth(int width);
    void setHeight(int height);
    void setScaleX(double scale);
    void setScaleY(double scale);

    void setKeepAspectRatio(bool enabled);
    void setSmoothTransformation(bool enabled);

    const QImage &currentImage() const;

signals:
    void imageUpdated();
    void errorOccurred(const QString &message);

private:
    void applyAndUpdate();

    BasicImageEditInterface *_editor;
    bool keepAspectRatio = true;
};

#endif // IMAGE_EDITOR_CONTEXT_H
