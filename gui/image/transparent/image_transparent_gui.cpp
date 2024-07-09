#include "image_transparent_gui.h"
#include "ui_image_transparent_gui.h"

#include "core/exception/invalid_argument_exception.h"
#include "core/image/transparent/image_transparent.h"

ImageTransparentGUI::ImageTransparentGUI(ImageTransparentInterface *imageTransparent,
                                         QWidget *parent)
    : GuiTool(parent)
    , ui(new Ui::ImageTransparentGUI)
    , imageTransparent(imageTransparent)
{
    ui->setupUi(this);

    connect(ui->colorMode,
            &QComboBox::currentTextChanged,
            this,
            &ImageTransparentGUI::onColorModeTextChanged);
    connect(ui->colorSample, &ColorSample::colorChanged, this, &ImageTransparentGUI::onColorChanged);
    connect(ui->transparencyValue,
            &QDoubleSpinBox::valueChanged,
            this,
            &ImageTransparentGUI::onTransparencyValueChanged);
    connect(ui->toleranceValue,
            &QDoubleSpinBox::valueChanged,
            this,
            &ImageTransparentGUI::onToleranceValueChanged);
    connect(ui->autoColorPick,
            &QCheckBox::checkStateChanged,
            this,
            &ImageTransparentGUI::onAutoColorPickCheckStateChanged);
    connect(ui->contiguousArea,
            &QCheckBox::checkStateChanged,
            this,
            &ImageTransparentGUI::onContiguousAreaCheckStateChanged);
}

ImageTransparentGUI::~ImageTransparentGUI()
{
    delete ui;
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

void ImageTransparentGUI::onAutoColorPickCheckStateChanged(Qt::CheckState state)
{
    switch (state) {
    case Qt::CheckState::Checked:
        break;
    case Qt::CheckState::Unchecked:
        break;
    default:
        // unreachable
        break;
    }
}

void ImageTransparentGUI::onColorChanged(const QColor &color)
{
    // 現状このUIは役に立たない。「自動で色を選択する/しない」みたいなフラグが必要。
}

void ImageTransparentGUI::onToleranceValueChanged(double tolerance)
{
    if (tolerance < 0.0 || 1.0 < tolerance)
        throw InvalidArgumentException<double>(tolerance, "tolerance must be in range [0.0, 1.0]");

    imageTransparent->tolerance = tolerance;

    // TODO: 画像を更新する
}

void ImageTransparentGUI::onTransparencyValueChanged(double transparency)
{
    if (transparency < 0.0 || 1.0 < transparency)
        throw InvalidArgumentException<double>(transparency,
                                               "transparency must be in range [0.0, 1.0]");

    imageTransparent->opacity = 255 * (1.0 - transparency);

    // TODO: 画像を更新する
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
