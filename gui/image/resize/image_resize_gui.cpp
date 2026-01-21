#include "image_resize_gui.h"

#include "core/image/resize/image_resize.h"
#include "ui_image_resize_gui.h"

#include <QMessageBox>
#include <QSignalBlocker>

ImageResizeGUI::ImageResizeGUI(ImageResizeInterface *imageResize, QWidget *parent)
    : GuiTool(parent), ui(new Ui::ImageResizeGUI), imageResize(imageResize)
{
    ui->setupUi(this);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageResize->parent() == nullptr)
        imageResize->setParent(this);

    // TODO: 共通化できないか?
    connect(ui->basicImageViewControl, &BasicImageViewControl::loadFileSelected, this,
            &ImageResizeGUI::onLoadImageSelected);
    connect(ui->basicImageViewControl, &BasicImageViewControl::saveFileSelected, this,
            &ImageResizeGUI::onSaveImageSelected);
    connect(ui->basicImageViewControl, &BasicImageViewControl::resetButtonClicked, this,
            &ImageResizeGUI::onResetButtonClicked);

    connect(ui->widthValue, &QSpinBox::valueChanged, this, &ImageResizeGUI::onWidthValueChanged);
    connect(ui->heightValue, &QSpinBox::valueChanged, this, &ImageResizeGUI::onHeightValueChanged);
    connect(ui->hScaleValue, &QDoubleSpinBox::valueChanged, this,
            &ImageResizeGUI::onHorizontalScaleChanged);
    connect(ui->vScaleValue, &QDoubleSpinBox::valueChanged, this,
            &ImageResizeGUI::onVerticalScaleChanged);

    connect(ui->keepAspectRatio, &QCheckBox::checkStateChanged, this,
            &ImageResizeGUI::onKeepAspectRatioChanged);
    connect(ui->smoothScaling, &QCheckBox::checkStateChanged, this,
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
    imageResize->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageResize->current()), true);
    updateUIValues();

    // TODO: load()の結果に応じて何かメッセージを出す
}

void ImageResizeGUI::onSaveImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    if (!imageResize->overwriteSave(path)) {
        QMessageBox::critical(this, tr("Save Failed"), tr("Failed to save the image."));
    }
}

void ImageResizeGUI::onResetButtonClicked()
{
    // NOTE: リセット処理。オリジナル画像は保持し、UIの拡大率などを初期値に戻す。
    imageResize->reset();

    ui->imageView->setPixmap(QPixmap::fromImage(imageResize->current()));
    updateUIValues();
}

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
