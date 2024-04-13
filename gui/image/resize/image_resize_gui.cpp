#include "image_resize_gui.h"
#include "ui_image_resize_gui.h"

#include <QSignalBlocker>
#include "core/image/resize/image_resize.h"

ImageResizeGUI::ImageResizeGUI(/*ImageResizeInterface *imageResize, */ QWidget *parent)
    : GuiTool(parent)
    , ui(new Ui::ImageResizeGUI)
//, imageResize(imageResize)
{
    ui->setupUi(this);
}

#if 0
    if (imageResize->parent() == nullptr)
        imageResize->setParent(this);

    // TODO: 共通化できないか?
    connect(ui->basicImageViewControl,
            &BasicImageViewControl::loadFileSelected,
            this,
            &ImageResizeGUI::onLoadImageSelected);
    connect(ui->basicImageViewControl,
            &BasicImageViewControl::saveFileSelected,
            this,
            &ImageResizeGUI::onSaveImageSelected);
    connect(ui->basicImageViewControl,
            &BasicImageViewControl::resetButtonClicked,
            this,
            &ImageResizeGUI::onResetButtonClicked);

    connect(ui->widthValue,
            &QSpinBox::editingFinished,
            this,
            &ImageResizeGUI::onWidthValueEditingFinished);
    connect(ui->heightValue,
            &QSpinBox::editingFinished,
            this,
            &ImageResizeGUI::onHeightValueEditingFinished);
    connect(ui->hScaleValue,
            &QDoubleSpinBox::editingFinished,
            this,
            &ImageResizeGUI::onHorizontalScaleEditingFinished);
    connect(ui->vScaleValue,
            &QDoubleSpinBox::editingFinished,
            this,
            &ImageResizeGUI::onVerticalScaleEditingFinished);
    // NOTE: Qt 6.7で stateChanged(int) → checkStateChanged(Qt::CheckState) に変更
    connect(ui->keepAspectRatio,
            &QCheckBox::stateChanged,
            this,
            &ImageResizeGUI::onKeepAspectRatioChanged);
    connect(ui->smoothScaling,
            &QCheckBox::stateChanged,
            this,
            &ImageResizeGUI::onSmoothTransformationChanged);
}

ImageResizeGUI::~ImageResizeGUI()
{
    delete ui;
}

void ImageResizeGUI::onLoadImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    bool result = imageResize->load(path);

    {
        const QSignalBlocker blockers[] = {
            QSignalBlocker(ui->widthValue),
            QSignalBlocker(ui->heightValue),
        };

        if (result) {
            ui->widthValue->setMinimum(1);
            ui->heightValue->setMinimum(1);
        } else {
            ui->widthValue->setMinimum(0);
            ui->heightValue->setMinimum(0);
        }
    }

    syncPixmap(true);
    syncValues(UpdateMode::LOAD, imageResize->keepAspectRatio);

    // TODO: load()の結果に応じて何かメッセージを出す
}

void ImageResizeGUI::onSaveImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    imageResize->save(path);

    // TODO: save()の結果に応じて何かメッセージを出す
}

void ImageResizeGUI::onResetButtonClicked()
{
    // NOTE: リセット処理。オリジナル画像は保持し、UIの拡大率などを初期値に戻す。
    const bool keepAspectRatio = imageResize->keepAspectRatio;
    imageResize->keepAspectRatio = true;

    imageResize->setScaleX(1.0);

    syncPixmap();
    {
        const QSignalBlocker blockers[] = {
            QSignalBlocker(ui->vScaleValue),
            QSignalBlocker(ui->hScaleValue),
        };
        ui->hScaleValue->setValue(100);
        ui->vScaleValue->setValue(100);
        syncValues(UpdateMode::X_SCALE_UPDATE, true);
    }

    // NOTE: syncValues()ではimageResize->keepAspectRatioを参照するので
    imageResize->keepAspectRatio = keepAspectRatio;
}

// FIXME: 画像が設定されていなくても動いてしまう
void ImageResizeGUI::onWidthValueEditingFinished()
{
    const int width = ui->widthValue->value();
    qDebug() << "width:" << width;

    if (width < 0) {
        qWarning() << "invalid width:" << width;
        return;
    }

    imageResize->setWidth(width);
    syncPixmap();
    syncValues(UpdateMode::WIDTH_UPDATE, imageResize->keepAspectRatio);
}

void ImageResizeGUI::onHeightValueEditingFinished()
{
    const int height = ui->heightValue->value();
    qDebug() << "height:" << height;

    if (height < 0) {
        qWarning() << "invalid height:" << height;
        return;
    }

    imageResize->setHeight(height);
    syncPixmap();
    syncValues(UpdateMode::HEIGHT_UPDATE, imageResize->keepAspectRatio);
}

void ImageResizeGUI::onHorizontalScaleEditingFinished()
{
    const double hScale = ui->hScaleValue->value();
    qDebug() << "hScale:" << hScale;

    if (hScale < 0) {
        qWarning() << "invalid horizontal scale" << hScale;
        return;
    }

    imageResize->setScaleX(hScale / 100.0);
    syncPixmap();
    syncValues(UpdateMode::X_SCALE_UPDATE, imageResize->keepAspectRatio);
}

void ImageResizeGUI::onVerticalScaleEditingFinished()
{
    const double vScale = ui->hScaleValue->value();
    qDebug() << "vScale:" << vScale;

    if (vScale < 0) {
        qWarning() << "invalid vertical scale" << vScale;
        return;
    }

    imageResize->setScaleY(vScale / 100.0);
    syncPixmap();
    syncValues(UpdateMode::Y_SCALE_UPDATE, imageResize->keepAspectRatio);
}

void ImageResizeGUI::onKeepAspectRatioChanged(int state)
{
    qDebug() << "state:" << state;

    switch (state) {
    case Qt::CheckState::Checked:
        imageResize->keepAspectRatio = true;
        break;
    case Qt::CheckState::Unchecked:
        imageResize->keepAspectRatio = false;
        break;
    default:
        // unreachable
        return;
    }

    // TODO: 無理やり更新させる
}

void ImageResizeGUI::onSmoothTransformationChanged(int state)
{
    qDebug() << "state:" << state;

    switch (state) {
    case Qt::CheckState::Checked:
        imageResize->smoothTransformation = true;
        break;
    case Qt::CheckState::Unchecked:
        imageResize->smoothTransformation = false;
        break;
    default:
        // unreachable
        return;
    }

    // TODO: 無理やり更新させる
}

// FIXME: 画像が設定されていなくても動いてしまう
void ImageResizeGUI::syncPixmap(bool reset)
{
    ui->imageView->setPixmap(imageResize->currentPixmap(), reset);
}

void ImageResizeGUI::syncValues(UpdateMode mode, bool keepAspectRatio)
{
    // NOTE: 値を変更するとsignalが発せられるので、それを防止する
    const QSignalBlocker blockers[] = {
        QSignalBlocker(ui->widthValue),
        QSignalBlocker(ui->heightValue),
        QSignalBlocker(ui->vScaleValue),
        QSignalBlocker(ui->hScaleValue),
    };

    const QSize currentSize = imageResize->currentSize();
    const QSize originalSize = imageResize->originalSize();

    if (mode != UpdateMode::WIDTH_UPDATE)
        ui->widthValue->setValue(currentSize.width());

    if (mode != UpdateMode::HEIGHT_UPDATE)
        ui->heightValue->setValue(currentSize.height());

    // NOTE: 縦横比固定かつ拡大率指定の場合は画像サイズから拡大率を算出しないようにする
    if (keepAspectRatio && mode == UpdateMode::X_SCALE_UPDATE) {
        ui->vScaleValue->setValue(ui->hScaleValue->value());
    } else if (keepAspectRatio && mode == UpdateMode::Y_SCALE_UPDATE) {
        ui->hScaleValue->setValue(ui->vScaleValue->value());
    } else {
        if (mode != UpdateMode::X_SCALE_UPDATE && 0 < originalSize.width())
            ui->hScaleValue->setValue(100.0 * currentSize.width() / originalSize.width());

        if (mode != UpdateMode::Y_SCALE_UPDATE && 0 < originalSize.height())
            ui->vScaleValue->setValue(100.0 * currentSize.height() / originalSize.height());
    }
}

#endif
