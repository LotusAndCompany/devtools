#include "image_editor_context.h"
#include "core/image/resize/image_resize.h"

ImageEditorContext::ImageEditorContext(BasicImageEditInterface *editor,
                                       QObject *parent)
    : QObject(parent)
    , _editor(editor)
{
}


bool ImageEditorContext::loadImage(const QString &path)
{
    if (!_editor->load(path)) {
        emit errorOccurred(tr("Failed to load image."));
        return false;
    }

    _editor->update();
    emit imageUpdated();
    return true;
}

bool ImageEditorContext::saveImage(const QString &path)
{
    if (!_editor->overwriteSave(path)) {
        emit errorOccurred(tr("Failed to save the image."));
        return false;
    }
    return true;
}

void ImageEditorContext::reset()
{
    _editor->reset();
    emit imageUpdated();
}

void ImageEditorContext::setWidth(int width)
{
    if (width < 0)
        return;

    // downcast は factory 側で保証されている前提
    auto resize = static_cast<ImageResize *>(_editor);
    resize->setWidth(width, keepAspectRatio);
    applyAndUpdate();
}

void ImageEditorContext::applyAndUpdate()
{
    if (_editor->isOutdated()) {
        _editor->update();
        emit imageUpdated();
    }
}

const QImage &ImageEditorContext::currentImage() const
{
    return _editor->current();
}

/*
void ImageResizeGUI::onWidthValueChanged(int width)
{
    qDebug() << "width:" << width;

    if (width < 0) {
        qWarning() << "invalid width:" << width;
        return;
    }

    if (imageResize->original().isNull())
        return;

    imageResize->setWidth(width, keepAspectRatio);
    imageResize->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageResize->current()));
    updateUIValues(UpdateMode::WIDTH_UPDATE);
}

void ImageResizeGUI::onHeightValueChanged(int height)
{
    qDebug() << "height:" << height;

    if (height < 0) {
        qWarning() << "invalid height:" << height;
        return;
    }

    if (imageResize->original().isNull())
        return;

    imageResize->setHeight(height, keepAspectRatio);
    imageResize->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageResize->current()));
    updateUIValues(UpdateMode::HEIGHT_UPDATE);
}

void ImageResizeGUI::onHorizontalScaleChanged(double hScale)
{
    qDebug() << "hScale:" << hScale;

    if (hScale < 0) {
        qWarning() << "invalid horizontal scale" << hScale;
        return;
    }

    if (imageResize->original().isNull())
        return;

    if (keepAspectRatio)
        imageResize->setScale(hScale / 100.0);
    else
        imageResize->setScaleX(hScale / 100.0);
    imageResize->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageResize->current()));
    updateUIValues(UpdateMode::X_SCALE_UPDATE);
}

void ImageResizeGUI::onVerticalScaleChanged(double vScale)
{
    qDebug() << "vScale:" << vScale;

    if (vScale < 0) {
        qWarning() << "invalid vertical scale" << vScale;
        return;
    }

    if (imageResize->original().isNull())
        return;

    if (keepAspectRatio)
        imageResize->setScale(vScale / 100.0);
    else
        imageResize->setScaleY(vScale / 100.0);
    imageResize->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageResize->current()));
    updateUIValues(UpdateMode::Y_SCALE_UPDATE);
}

void ImageResizeGUI::onKeepAspectRatioChanged(Qt::CheckState state)
{
    qDebug() << "state:" << state;

    switch (state) {
    case Qt::CheckState::Checked:
        keepAspectRatio = true;
        break;
    case Qt::CheckState::Unchecked:
        keepAspectRatio = false;
        break;
    default:
        // unreachable
        return;
    }

    // TODO: 無理やり更新させる
}

void ImageResizeGUI::onSmoothTransformationChanged(Qt::CheckState state)
{
    qDebug() << "state:" << state;

    switch (state) {
    case Qt::CheckState::Checked:
        imageResize->setSmoothTransformationEnabled(true);
        break;
    case Qt::CheckState::Unchecked:
        imageResize->setSmoothTransformationEnabled(false);
        break;
    default:
        // unreachable
        return;
    }

    // TODO: 無理やり更新させる
}

void ImageResizeGUI::updateUIValues(UpdateMode mode)
{
    // NOTE: 値を変更するとsignalが発せられるので、それを防止する
    const QSignalBlocker blockers[] = {
        QSignalBlocker(ui->widthValue),
        QSignalBlocker(ui->heightValue),
        QSignalBlocker(ui->vScaleValue),
        QSignalBlocker(ui->hScaleValue),
    };

    if (imageResize->original().isNull()) {
        ui->widthValue->setValue(0);
        ui->heightValue->setValue(0);
        ui->hScaleValue->setValue(100.0);
        ui->vScaleValue->setValue(100.0);

        return;
    }

    const auto size = imageResize->computedSize();
    const double scaleX = imageResize->computedScaleX();
    const double scaleY = imageResize->computedScaleY();

    if (mode != UpdateMode::WIDTH_UPDATE)
        ui->widthValue->setValue(size.width());

    if (mode != UpdateMode::HEIGHT_UPDATE)
        ui->heightValue->setValue(size.height());

    // NOTE: 縦横比固定かつ拡大率指定の場合は画像サイズから拡大率を算出しないようにする
    if (keepAspectRatio && mode == UpdateMode::X_SCALE_UPDATE) {
        ui->vScaleValue->setValue(ui->hScaleValue->value());
    } else if (keepAspectRatio && mode == UpdateMode::Y_SCALE_UPDATE) {
        ui->hScaleValue->setValue(ui->vScaleValue->value());
    } else {
        if (mode != UpdateMode::X_SCALE_UPDATE)
            ui->hScaleValue->setValue(100.0 * scaleX);

        if (mode != UpdateMode::Y_SCALE_UPDATE)
            ui->vScaleValue->setValue(100.0 * scaleY);
    }
}
*/
