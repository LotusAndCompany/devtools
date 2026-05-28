#include "image_transparent_gui.h"

#include "core/exception/invalid_argument_exception.h"
#include "core/image/transparent/image_transparent.h"
#include "gui/image/basic/control.h"
#include "gui/image/transparent/color_sample.h"
#include "gui/image/transparent/image_view_for_image_transparent.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpacerItem>
#include <QVBoxLayout>

namespace {
void buildFormFields(Ui::ImageTransparentGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *form = new QFormLayout();
    form->setSizeConstraint(QLayout::SetDefaultConstraint);

    ui->colorMode = new QComboBox(parent);
    ui->colorMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    // NOTE: 表示文字列ではなく itemData の QColor::Spec で識別する
    ui->colorMode->addItem("RGB", static_cast<int>(QColor::Spec::Rgb));
    ui->colorMode->addItem("HSL", static_cast<int>(QColor::Spec::Hsl));
    ui->colorMode->addItem("HSV", static_cast<int>(QColor::Spec::Hsv));
    auto *colorModeLabel = new QLabel(ImageTransparentGUI::tr("Color mode:"), parent);
    colorModeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    form->addRow(colorModeLabel, ui->colorMode);

    ui->colorSample = new ColorSample(parent);
    ui->colorSample->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->colorSample->setMinimumSize(32, 32);
    form->addRow(new QLabel(ImageTransparentGUI::tr("Color:"), parent), ui->colorSample);

    ui->toleranceValue = new QDoubleSpinBox(parent);
    ui->toleranceValue->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    ui->toleranceValue->setMaximum(1.0);
    ui->toleranceValue->setSingleStep(0.05);
    ui->toleranceValue->setValue(0.1);
    form->addRow(new QLabel(ImageTransparentGUI::tr("Tolerance:"), parent), ui->toleranceValue);

    ui->transparencyValue = new QDoubleSpinBox(parent);
    ui->transparencyValue->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    ui->transparencyValue->setMaximum(1.0);
    ui->transparencyValue->setSingleStep(0.05);
    ui->transparencyValue->setValue(1.0);
    form->addRow(new QLabel(ImageTransparentGUI::tr("Transparency:"), parent),
                 ui->transparencyValue);

    layout->addLayout(form);
}

void buildUiArea(Ui::ImageTransparentGUI *ui, QWidget *parent, QHBoxLayout *rootLayout)
{
    auto *uiArea = new QFrame(parent);
    uiArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    uiArea->setMinimumWidth(240);
    uiArea->setFrameShape(QFrame::StyledPanel);
    uiArea->setFrameShadow(QFrame::Raised);

    auto *layout = new QVBoxLayout(uiArea);
    layout->setContentsMargins(12, 12, 12, 12);

    buildFormFields(ui, uiArea, layout);

    ui->contiguousArea = new QCheckBox(ImageTransparentGUI::tr("Only contiguous area"), uiArea);
    ui->contiguousArea->setChecked(true);
    layout->addWidget(ui->contiguousArea);

    layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    ui->control = new BasicImageViewControl(uiArea);
    layout->addWidget(ui->control);

    rootLayout->addWidget(uiArea);
}
} // namespace

ImageTransparentGUI::ImageTransparentGUI(ImageTransparentInterface *imageTransparent,
                                         QWidget *parent)
    : GuiTool(parent), ui(new Ui::ImageTransparentGUI), imageTransparent(imageTransparent)
{
    resize(480, 300);

    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    ui->imageView = new ImageViewForImageTransparent(this);
    ui->imageView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->imageView->setMinimumWidth(240);
    rootLayout->addWidget(ui->imageView);

    buildUiArea(ui, this, rootLayout);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageTransparent->parent() == nullptr) {
        imageTransparent->setParent(this);
    }

    connect(ui->imageView, &BasicImageView::loadFileSelected, ui->control,
            &BasicImageViewControl::loadFileSelected);
    connect(ui->control, &BasicImageViewControl::loadFileSelected, this,
            &ImageTransparentGUI::onLoadImageSelected);
    connect(ui->control, &BasicImageViewControl::saveFileSelected, this,
            &ImageTransparentGUI::onSaveImageSelected);
    connect(ui->control, &BasicImageViewControl::resetButtonClicked, this,
            &ImageTransparentGUI::onResetButtonClicked);

    connect(ui->colorMode, &QComboBox::currentIndexChanged, this,
            &ImageTransparentGUI::onColorModeIndexChanged);
    connect(ui->imageView, &ImageViewForImageTransparent::pixelSelected, this,
            &ImageTransparentGUI::onPixelSelected);
    connect(ui->transparencyValue, &QDoubleSpinBox::valueChanged, this,
            &ImageTransparentGUI::onTransparencyValueChanged);
    connect(ui->toleranceValue, &QDoubleSpinBox::valueChanged, this,
            &ImageTransparentGUI::onToleranceValueChanged);
    connect(ui->contiguousArea, &QCheckBox::checkStateChanged, this,
            &ImageTransparentGUI::onContiguousAreaCheckStateChanged);
}

ImageTransparentGUI::~ImageTransparentGUI()
{
    delete ui;
}

void ImageTransparentGUI::onLoadImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    imageTransparent->load(path);
    imageTransparent->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageTransparent->current()), true);

    // TODO: load()の結果に応じて何かメッセージを出す
}

void ImageTransparentGUI::onSaveImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    if (!imageTransparent->overwriteSave(path)) {
        QMessageBox::critical(this, tr("Save Failed"), tr("Failed to save the image."));
    }
}

void ImageTransparentGUI::onResetButtonClicked()
{
    // NOTE: リセット処理。オリジナル画像は保持し、編集内容を初期値に戻す。
    imageTransparent->reset();

    ui->imageView->setPixmap(QPixmap::fromImage(imageTransparent->current()));
}

void ImageTransparentGUI::onColorModeIndexChanged(int index)
{
    const QVariant data = ui->colorMode->itemData(index);
    if (!data.isValid()) {
        throw InvalidArgumentException<int>(index,
                                            "color mode item has no associated QColor::Spec");
    }
    imageTransparent->colorSpec = static_cast<QColor::Spec>(data.toInt());
}

void ImageTransparentGUI::onPixelSelected(const QPoint &point, const QColor &color)
{
    qDebug() << "point:" << point << ", color:" << color;
    ui->colorSample->setColor(color);

    if (onlyContiguousArea) {
        imageTransparent->addTransparentPixel(point);
    } else {
        imageTransparent->addTransparentColor(color);
    }

    ui->imageView->setPixmap(QPixmap::fromImage(imageTransparent->current()));
}

void ImageTransparentGUI::onToleranceValueChanged(double tolerance)
{
    qDebug() << "tolerance:" << tolerance;

    if (tolerance < 0.0 || 1.0 < tolerance) {
        throw InvalidArgumentException<double>(tolerance, "tolerance must be in range [0.0, 1.0]");
    }

    imageTransparent->tolerance = tolerance;
}

void ImageTransparentGUI::onTransparencyValueChanged(double transparency)
{
    qDebug() << "transparency:" << transparency;

    if (transparency < 0.0 || 1.0 < transparency) {
        throw InvalidArgumentException<double>(transparency,
                                               "transparency must be in range [0.0, 1.0]");
    }

    imageTransparent->opacity = static_cast<uint8_t>(255 * (1.0 - transparency));
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
