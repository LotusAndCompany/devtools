#include "image_transparent_gui.h"
#include "gui/image/transparent/image_view_for_image_transparent.h"
#include "ui_image_transparent_gui.h"

#include "core/exception/invalid_argument_exception.h"
#include "core/image/transparent/image_transparent.h"

#include <QMessageBox>

ImageTransparentGUI::ImageTransparentGUI(ImageTransparentInterface *imageTransparent,
                                         QWidget *parent)
    : GuiTool(parent)
    , ui(new Ui::ImageTransparentGUI)
    , imageTransparent(imageTransparent)
{
    ui->setupUi(this);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageTransparent->parent() == nullptr)
        imageTransparent->setParent(this);

    connect(ui->control,
            &BasicImageViewControl::loadFileSelected,
            this,
            &ImageTransparentGUI::onLoadImageSelected);
    connect(ui->control,
            &BasicImageViewControl::saveFileSelected,
            this,
            &ImageTransparentGUI::onSaveImageSelected);
    connect(ui->control,
            &BasicImageViewControl::resetButtonClicked,
            this,
            &ImageTransparentGUI::onResetButtonClicked);

    connect(ui->colorMode,
            &QComboBox::currentTextChanged,
            this,
            &ImageTransparentGUI::onColorModeTextChanged);
    connect(ui->imageView,
            &ImageViewForImageTransparent::pixelSelected,
            this,
            &ImageTransparentGUI::onPixelSelected);
    connect(ui->transparencyValue,
            &QDoubleSpinBox::valueChanged,
            this,
            &ImageTransparentGUI::onTransparencyValueChanged);
    connect(ui->toleranceValue,
            &QDoubleSpinBox::valueChanged,
            this,
            &ImageTransparentGUI::onToleranceValueChanged);
    connect(ui->contiguousArea,
            &QCheckBox::checkStateChanged,
            this,
            &ImageTransparentGUI::onContiguousAreaCheckStateChanged);
}

ImageTransparentGUI::~ImageTransparentGUI()
{
    delete ui;
}

void ImageTransparentGUI::onLoadImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    bool result = imageTransparent->load(path);
    imageTransparent->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageTransparent->current()), true);

    // TODO: load()の結果に応じて何かメッセージを出す
}

void ImageTransparentGUI::onSaveImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    if (imageTransparent->overwriteSave(path)) {
        QMessageBox::critical(this, tr("Save Failed"), tr("Failed to save the image."));
    }
}

void ImageTransparentGUI::onResetButtonClicked()
{
    // NOTE: リセット処理。オリジナル画像は保持し、編集内容を初期値に戻す。
    imageTransparent->reset();

    ui->imageView->setPixmap(QPixmap::fromImage(imageTransparent->current()));
}

void ImageTransparentGUI::onColorModeTextChanged(const QString &mode)
{
    if (mode == "RGB") {
        imageTransparent->colorSpec = QColor::Spec::Rgb;
    } else if (mode == "HSV") {
        imageTransparent->colorSpec = QColor::Spec::Hsv;
    } else if (mode == "HSL") {
        imageTransparent->colorSpec = QColor::Spec::Hsl;
    } else {
        throw InvalidArgumentException<QString>(mode, "mode must be RGB, HSV or HSL");
    }
}

void ImageTransparentGUI::onPixelSelected(const QPoint &point, const QColor &color)
{
    qDebug() << "point:" << point << ", color:" << color;
    ui->colorSample->setColor(color);

    if (onlyContiguousArea)
        imageTransparent->addTransparentPixel(point);
    else
        imageTransparent->addTransparentColor(color);

    ui->imageView->setPixmap(QPixmap::fromImage(imageTransparent->current()));
}

void ImageTransparentGUI::onToleranceValueChanged(double tolerance)
{
    qDebug() << "tolerance:" << tolerance;

    if (tolerance < 0.0 || 1.0 < tolerance)
        throw InvalidArgumentException<double>(tolerance, "tolerance must be in range [0.0, 1.0]");

    imageTransparent->tolerance = tolerance;
}

void ImageTransparentGUI::onTransparencyValueChanged(double transparency)
{
    qDebug() << "transparency:" << transparency;

    if (transparency < 0.0 || 1.0 < transparency)
        throw InvalidArgumentException<double>(transparency,
                                               "transparency must be in range [0.0, 1.0]");

    imageTransparent->opacity = 255 * (1.0 - transparency);
}

void ImageTransparentGUI::onContiguousAreaCheckStateChanged(Qt::CheckState state)
{
    switch (state) {
    case Qt::CheckState::Checked:
        onlyContiguousArea = true;
        break;
    case Qt::CheckState::Unchecked:
        onlyContiguousArea = false;
        break;
    default:
        // unreachable
        break;
    }
}
