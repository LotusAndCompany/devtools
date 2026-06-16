#include "image_tools_unified_gui.h"

#include "gui/image/basic/control.h"
#include "gui/image/transparent/color_sample.h"
#include "gui/image/transparent/image_view_for_image_transparent.h"

#include <QAbstractButton>
#include <QAbstractSpinBox>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QQueue>
#include <QRadioButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpacerItem>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QVector3D>
#include <QVector>

#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr double M_TAU = 2.0 * M_PI;

struct DivisionPlan
{
    int xCount = 0;
    int yCount = 0;
    double stepW = 0.0;
    double stepH = 0.0;
};

int countByCellSizeForPlan(int source, int cell, bool ignoreRemainders)
{
    if (source <= 0 || cell <= 0) {
        return 0;
    }

    const int base = source / cell;
    if (ignoreRemainders || (source % cell == 0)) {
        return base;
    }

    return base + 1;
}

QSpinBox *buildPixelSpinBox(QWidget *parent)
{
    auto *spin = new QSpinBox(parent);
    spin->setSuffix("px");
    spin->setMinimum(1);
    spin->setMaximum(16777215);
    spin->setSingleStep(1);
    spin->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    spin->setDisplayIntegerBase(10);
    return spin;
}

QDoubleSpinBox *buildScaleSpinBox(QWidget *parent)
{
    auto *spin = new QDoubleSpinBox(parent);
    spin->setSuffix("%");
    spin->setDecimals(2);
    spin->setMinimum(0.1);
    spin->setMaximum(1000.0);
    spin->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    spin->setValue(100.0);
    return spin;
}

QPushButton *buildActionButton(QWidget *parent, const QString &text, const QString &iconName)
{
    auto *button = new QPushButton(QIcon::fromTheme(iconName), text, parent);
    button->setStyleSheet("text-align: left;");
    return button;
}

void buildResizeSection(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *frame = new QFrame(parent);
    ui->resizeSection = frame;
    frame->setFrameShape(QFrame::StyledPanel);
    auto *box = new QVBoxLayout(frame);
    ui->resizeSectionToggle = new QCheckBox(ImageToolsUnifiedGUI::tr("Resize"), frame);
    ui->resizeSectionToggle->setChecked(true);
    box->addWidget(ui->resizeSectionToggle);

    ui->resizeSectionBody = new QWidget(frame);
    auto *bodyLayout = new QVBoxLayout(ui->resizeSectionBody);
    bodyLayout->setContentsMargins(0, 0, 0, 0);

    auto *form = new QFormLayout();
    ui->widthValue = buildPixelSpinBox(ui->resizeSectionBody);
    ui->heightValue = buildPixelSpinBox(ui->resizeSectionBody);
    ui->hScaleValue = buildScaleSpinBox(ui->resizeSectionBody);
    ui->vScaleValue = buildScaleSpinBox(ui->resizeSectionBody);
    form->addRow(ImageToolsUnifiedGUI::tr("Width:"), ui->widthValue);
    form->addRow(ImageToolsUnifiedGUI::tr("Height:"), ui->heightValue);
    form->addRow(ImageToolsUnifiedGUI::tr("Horizontal Scale:"), ui->hScaleValue);
    form->addRow(ImageToolsUnifiedGUI::tr("Vertical Scale:"), ui->vScaleValue);
    bodyLayout->addLayout(form);

    ui->keepAspectRatio =
        new QCheckBox(ImageToolsUnifiedGUI::tr("Keep aspect ratio"), ui->resizeSectionBody);
    ui->smoothScaling =
        new QCheckBox(ImageToolsUnifiedGUI::tr("Smooth scaling"), ui->resizeSectionBody);
    bodyLayout->addWidget(ui->keepAspectRatio);
    bodyLayout->addWidget(ui->smoothScaling);

    ui->applyResizeBySizeButton =
        new QPushButton(ImageToolsUnifiedGUI::tr("Apply Size"), ui->resizeSectionBody);
    ui->applyResizeByScaleButton =
        new QPushButton(ImageToolsUnifiedGUI::tr("Apply Scale"), ui->resizeSectionBody);
    bodyLayout->addWidget(ui->applyResizeBySizeButton);
    bodyLayout->addWidget(ui->applyResizeByScaleButton);

    box->addWidget(ui->resizeSectionBody);

    layout->addWidget(frame);
}

void buildTransformSection(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *frame = new QFrame(parent);
    ui->transformSection = frame;
    frame->setFrameShape(QFrame::StyledPanel);
    auto *box = new QVBoxLayout(frame);
    ui->transformSectionToggle = new QCheckBox(ImageToolsUnifiedGUI::tr("Rotate / Flip"), frame);
    ui->transformSectionToggle->setChecked(true);
    box->addWidget(ui->transformSectionToggle);

    ui->transformSectionBody = new QWidget(frame);
    auto *bodyLayout = new QVBoxLayout(ui->transformSectionBody);
    bodyLayout->setContentsMargins(0, 0, 0, 0);

    ui->rotateLeftButton =
        buildActionButton(ui->transformSectionBody,
                          ImageToolsUnifiedGUI::tr("Rotate Anti-clockwise"), "anticlockwise");
    ui->rotateRightButton = buildActionButton(
        ui->transformSectionBody, ImageToolsUnifiedGUI::tr("Rotate Clockwise"), "clockwise");
    ui->flipHorizontalButton = buildActionButton(
        ui->transformSectionBody, ImageToolsUnifiedGUI::tr("Flip Horizontal"), "flip_horizontal");
    ui->flipVerticalButton = buildActionButton(
        ui->transformSectionBody, ImageToolsUnifiedGUI::tr("Flip Vertical"), "flip_vertical");

    bodyLayout->addWidget(ui->rotateLeftButton);
    bodyLayout->addWidget(ui->rotateRightButton);
    bodyLayout->addWidget(ui->flipHorizontalButton);
    bodyLayout->addWidget(ui->flipVerticalButton);

    box->addWidget(ui->transformSectionBody);

    layout->addWidget(frame);
}

void buildTransparentSection(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *frame = new QFrame(parent);
    ui->transparentSection = frame;
    frame->setFrameShape(QFrame::StyledPanel);
    auto *box = new QVBoxLayout(frame);
    ui->transparentSectionToggle =
        new QCheckBox(ImageToolsUnifiedGUI::tr("Transparent (click image)"), frame);
    ui->transparentSectionToggle->setChecked(true);
    box->addWidget(ui->transparentSectionToggle);

    ui->transparentSectionBody = new QWidget(frame);
    auto *bodyLayout = new QVBoxLayout(ui->transparentSectionBody);
    bodyLayout->setContentsMargins(0, 0, 0, 0);

    auto *form = new QFormLayout();
    ui->colorMode = new QComboBox(ui->transparentSectionBody);
    ui->colorMode->addItem("RGB", static_cast<int>(QColor::Spec::Rgb));
    ui->colorMode->addItem("HSL", static_cast<int>(QColor::Spec::Hsl));
    ui->colorMode->addItem("HSV", static_cast<int>(QColor::Spec::Hsv));
    form->addRow(ImageToolsUnifiedGUI::tr("Color mode:"), ui->colorMode);

    ui->colorSample = new ColorSample(ui->transparentSectionBody);
    ui->colorSample->setMinimumSize(24, 24);
    form->addRow(ImageToolsUnifiedGUI::tr("Color:"), ui->colorSample);

    ui->toleranceValue = new QDoubleSpinBox(ui->transparentSectionBody);
    ui->toleranceValue->setMinimum(0.0);
    ui->toleranceValue->setMaximum(1.0);
    ui->toleranceValue->setSingleStep(0.05);
    ui->toleranceValue->setValue(0.1);
    form->addRow(ImageToolsUnifiedGUI::tr("Tolerance:"), ui->toleranceValue);

    ui->transparencyValue = new QDoubleSpinBox(ui->transparentSectionBody);
    ui->transparencyValue->setMinimum(0.0);
    ui->transparencyValue->setMaximum(1.0);
    ui->transparencyValue->setSingleStep(0.05);
    ui->transparencyValue->setValue(1.0);
    form->addRow(ImageToolsUnifiedGUI::tr("Transparency:"), ui->transparencyValue);
    bodyLayout->addLayout(form);

    ui->contiguousArea =
        new QCheckBox(ImageToolsUnifiedGUI::tr("Only contiguous area"), ui->transparentSectionBody);
    ui->contiguousArea->setChecked(true);
    bodyLayout->addWidget(ui->contiguousArea);

    box->addWidget(ui->transparentSectionBody);

    layout->addWidget(frame);
}

void buildDivisionSection(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *frame = new QFrame(parent);
    ui->divisionSection = frame;
    frame->setFrameShape(QFrame::StyledPanel);
    auto *box = new QVBoxLayout(frame);
    ui->divisionSectionToggle = new QCheckBox(ImageToolsUnifiedGUI::tr("Division"), frame);
    ui->divisionSectionToggle->setChecked(true);
    box->addWidget(ui->divisionSectionToggle);

    ui->divisionSectionBody = new QWidget(frame);
    auto *bodyLayout = new QVBoxLayout(ui->divisionSectionBody);
    bodyLayout->setContentsMargins(0, 0, 0, 0);

    auto *sizeRow = new QHBoxLayout();
    sizeRow->addWidget(
        new QLabel(ImageToolsUnifiedGUI::tr("Image size:"), ui->divisionSectionBody));
    ui->sizeLabel = new QLabel(ImageToolsUnifiedGUI::tr("0 x 0"), ui->divisionSectionBody);
    sizeRow->addWidget(ui->sizeLabel);
    bodyLayout->addLayout(sizeRow);

    ui->useDivisionButton =
        new QRadioButton(ImageToolsUnifiedGUI::tr("Division"), ui->divisionSectionBody);
    ui->useDivisionButton->setChecked(true);
    ui->useSizeButton = new QRadioButton(ImageToolsUnifiedGUI::tr("Size"), ui->divisionSectionBody);
    bodyLayout->addWidget(ui->useDivisionButton);
    bodyLayout->addWidget(ui->useSizeButton);

    auto *grid = new QGridLayout();
    ui->hDivValue = new QSpinBox(ui->divisionSectionBody);
    ui->hDivValue->setMinimum(1);
    ui->vDivValue = new QSpinBox(ui->divisionSectionBody);
    ui->vDivValue->setMinimum(1);
    ui->cellWidthValue = buildPixelSpinBox(ui->divisionSectionBody);
    ui->cellHeightValue = buildPixelSpinBox(ui->divisionSectionBody);
    ui->cellWidthValue->setEnabled(false);
    ui->cellHeightValue->setEnabled(false);

    grid->addWidget(
        new QLabel(ImageToolsUnifiedGUI::tr("Horizontal division:"), ui->divisionSectionBody), 0,
        0);
    grid->addWidget(ui->hDivValue, 0, 1);
    grid->addWidget(
        new QLabel(ImageToolsUnifiedGUI::tr("Vertical division:"), ui->divisionSectionBody), 1, 0);
    grid->addWidget(ui->vDivValue, 1, 1);
    grid->addWidget(new QLabel(ImageToolsUnifiedGUI::tr("Cell width:"), ui->divisionSectionBody), 2,
                    0);
    grid->addWidget(ui->cellWidthValue, 2, 1);
    grid->addWidget(new QLabel(ImageToolsUnifiedGUI::tr("Cell height:"), ui->divisionSectionBody),
                    3, 0);
    grid->addWidget(ui->cellHeightValue, 3, 1);
    bodyLayout->addLayout(grid);

    ui->divisionModeButtonGroup = new QButtonGroup(parent);
    ui->divisionModeButtonGroup->addButton(ui->useDivisionButton);
    ui->divisionModeButtonGroup->addButton(ui->useSizeButton);

    ui->ignoreRemainders =
        new QCheckBox(ImageToolsUnifiedGUI::tr("Ignore remainders"), ui->divisionSectionBody);
    ui->ignoreRemainders->setChecked(true);
    bodyLayout->addWidget(ui->ignoreRemainders);

    ui->saveDividedButton =
        new QPushButton(ImageToolsUnifiedGUI::tr("Save Divided Images"), ui->divisionSectionBody);
    bodyLayout->addWidget(ui->saveDividedButton);

    box->addWidget(ui->divisionSectionBody);

    layout->addWidget(frame);
}

void buildUiArea(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QHBoxLayout *rootLayout)
{
    ui->toolScrollArea = new QScrollArea(parent);
    ui->toolScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->toolScrollArea->setMinimumWidth(360);
    ui->toolScrollArea->setWidgetResizable(true);
    ui->toolScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *uiArea = new QWidget(ui->toolScrollArea);
    auto *layout = new QVBoxLayout(uiArea);
    layout->setContentsMargins(8, 8, 8, 8);

    buildResizeSection(ui, uiArea, layout);
    buildTransformSection(ui, uiArea, layout);
    buildTransparentSection(ui, uiArea, layout);
    buildDivisionSection(ui, uiArea, layout);

    layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    ui->control = new BasicImageViewControl(uiArea);
    layout->addWidget(ui->control);

    ui->toolScrollArea->setWidget(uiArea);
    rootLayout->addWidget(ui->toolScrollArea);
}

DivisionPlan buildDivisionPlan(const Ui::ImageToolsUnifiedGUI *ui, bool useDivisionMode,
                               int sourceW, int sourceH)
{
    DivisionPlan plan;
    if (useDivisionMode) {
        plan.xCount = std::max(1, ui->hDivValue->value());
        plan.yCount = std::max(1, ui->vDivValue->value());
        plan.stepW = static_cast<double>(sourceW) / plan.xCount;
        plan.stepH = static_cast<double>(sourceH) / plan.yCount;
        return plan;
    }

    const int cellW = std::max(1, ui->cellWidthValue->value());
    const int cellH = std::max(1, ui->cellHeightValue->value());
    plan.xCount = countByCellSizeForPlan(sourceW, cellW, ui->ignoreRemainders->isChecked());
    plan.yCount = countByCellSizeForPlan(sourceH, cellH, ui->ignoreRemainders->isChecked());
    plan.stepW = cellW;
    plan.stepH = cellH;
    return plan;
}

QRect divisionRect(const Ui::ImageToolsUnifiedGUI *ui, const DivisionPlan &plan, int x, int y,
                   int sourceW, int sourceH)
{
    int left = static_cast<int>(std::floor(x * plan.stepW));
    int top = static_cast<int>(std::floor(y * plan.stepH));
    int right = static_cast<int>(std::floor((x + 1) * plan.stepW));
    int bottom = static_cast<int>(std::floor((y + 1) * plan.stepH));

    if (!ui->ignoreRemainders->isChecked()) {
        if (x == plan.xCount - 1) {
            right = sourceW;
        }
        if (y == plan.yCount - 1) {
            bottom = sourceH;
        }
    }

    left = std::clamp(left, 0, sourceW);
    top = std::clamp(top, 0, sourceH);
    right = std::clamp(right, 0, sourceW);
    bottom = std::clamp(bottom, 0, sourceH);

    if (right <= left || bottom <= top) {
        return {};
    }

    return {left, top, right - left, bottom - top};
}
} // namespace

ImageToolsUnifiedGUI::ImageToolsUnifiedGUI(QWidget *parent)
    : GuiTool(parent), ui(new Ui::ImageToolsUnifiedGUI)
{
    resize(520, 360);

    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    ui->imageView = new ImageViewForImageTransparent(this);
    ui->imageView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    rootLayout->addWidget(ui->imageView);

    buildUiArea(ui, this, rootLayout);

    connect(ui->imageView, &ImageViewForImageTransparent::loadFileSelected, ui->control,
            &BasicImageViewControl::loadFileSelected);
    connect(ui->control, &BasicImageViewControl::loadFileSelected, this,
            &ImageToolsUnifiedGUI::onLoadImageSelected);
    connect(ui->control, &BasicImageViewControl::saveFileSelected, this,
            &ImageToolsUnifiedGUI::onSaveImageSelected);
    connect(ui->control, &BasicImageViewControl::resetButtonClicked, this,
            &ImageToolsUnifiedGUI::onResetButtonClicked);

    connect(ui->applyResizeBySizeButton, &QPushButton::clicked, this,
            &ImageToolsUnifiedGUI::onApplyResizeBySizeClicked);
    connect(ui->applyResizeByScaleButton, &QPushButton::clicked, this,
            &ImageToolsUnifiedGUI::onApplyResizeByScaleClicked);
    connect(ui->keepAspectRatio, &QCheckBox::checkStateChanged, this,
            &ImageToolsUnifiedGUI::onKeepAspectRatioChanged);

    connect(ui->rotateLeftButton, &QPushButton::clicked, this,
            &ImageToolsUnifiedGUI::onRotateLeftButtonClicked);
    connect(ui->rotateRightButton, &QPushButton::clicked, this,
            &ImageToolsUnifiedGUI::onRotateRightButtonClicked);
    connect(ui->flipHorizontalButton, &QPushButton::clicked, this,
            &ImageToolsUnifiedGUI::onFlipHorizontalButtonClicked);
    connect(ui->flipVerticalButton, &QPushButton::clicked, this,
            &ImageToolsUnifiedGUI::onFlipVerticalButtonClicked);

    connect(ui->colorMode, &QComboBox::currentIndexChanged, this,
            &ImageToolsUnifiedGUI::onColorModeIndexChanged);
    connect(ui->imageView, &ImageViewForImageTransparent::pixelSelected, this,
            &ImageToolsUnifiedGUI::onPixelSelected);
    connect(ui->toleranceValue, &QDoubleSpinBox::valueChanged, this,
            &ImageToolsUnifiedGUI::onToleranceValueChanged);
    connect(ui->transparencyValue, &QDoubleSpinBox::valueChanged, this,
            &ImageToolsUnifiedGUI::onTransparencyValueChanged);
    connect(ui->contiguousArea, &QCheckBox::checkStateChanged, this,
            &ImageToolsUnifiedGUI::onContiguousAreaCheckStateChanged);

    connect(ui->divisionModeButtonGroup, &QButtonGroup::buttonClicked, this,
            &ImageToolsUnifiedGUI::onDivisionModeClicked);
    connect(ui->saveDividedButton, &QPushButton::clicked, this,
            &ImageToolsUnifiedGUI::onSaveDividedButtonClicked);

    connect(ui->resizeSectionToggle, &QCheckBox::checkStateChanged, this,
            &ImageToolsUnifiedGUI::onSectionVisibilityChanged);
    connect(ui->transformSectionToggle, &QCheckBox::checkStateChanged, this,
            &ImageToolsUnifiedGUI::onSectionVisibilityChanged);
    connect(ui->transparentSectionToggle, &QCheckBox::checkStateChanged, this,
            &ImageToolsUnifiedGUI::onSectionVisibilityChanged);
    connect(ui->divisionSectionToggle, &QCheckBox::checkStateChanged, this,
            &ImageToolsUnifiedGUI::onSectionVisibilityChanged);

    onSectionVisibilityChanged();
}

ImageToolsUnifiedGUI::~ImageToolsUnifiedGUI()
{
    delete ui;
}

bool ImageToolsUnifiedGUI::hasImage() const
{
    return !currentImage.isNull();
}

void ImageToolsUnifiedGUI::refreshPreview(bool resetScale)
{
    ui->imageView->setPixmap(QPixmap::fromImage(currentImage), resetScale);
    refreshSizeInputs();
}

void ImageToolsUnifiedGUI::refreshSizeInputs()
{
    const QSignalBlocker widthBlocker(ui->widthValue);
    const QSignalBlocker heightBlocker(ui->heightValue);

    const QSize size = currentImage.size();
    ui->widthValue->setValue(std::max(1, size.width()));
    ui->heightValue->setValue(std::max(1, size.height()));
    ui->sizeLabel->setText(tr("%1 x %2").arg(size.width()).arg(size.height()));

    ui->hDivValue->setMaximum(std::max(1, size.width()));
    ui->vDivValue->setMaximum(std::max(1, size.height()));
    ui->cellWidthValue->setMaximum(std::max(1, size.width()));
    ui->cellHeightValue->setMaximum(std::max(1, size.height()));
}

void ImageToolsUnifiedGUI::onLoadImageSelected(const QString &path)
{
    const QImage loaded(path);
    if (loaded.isNull()) {
        QMessageBox::critical(this, tr("Load Failed"), tr("Failed to load the image."));
        return;
    }

    loadedFilePath = path;
    originalImage = loaded.convertToFormat(QImage::Format_RGBA8888);
    currentImage = originalImage;

    refreshPreview(true);
}

void ImageToolsUnifiedGUI::onSaveImageSelected(const QString &path)
{
    if (!hasImage()) {
        return;
    }

    if (!currentImage.save(path)) {
        QMessageBox::critical(this, tr("Save Failed"), tr("Failed to save the image."));
    }
}

void ImageToolsUnifiedGUI::onResetButtonClicked()
{
    if (originalImage.isNull()) {
        return;
    }

    currentImage = originalImage;
    refreshPreview();
}

void ImageToolsUnifiedGUI::applyResize(const QSize &targetSize)
{
    if (!hasImage()) {
        return;
    }
    if (targetSize.isEmpty()) {
        return;
    }

    currentImage = currentImage.scaled(
        targetSize, Qt::IgnoreAspectRatio,
        ui->smoothScaling->isChecked() ? Qt::SmoothTransformation : Qt::FastTransformation);
    refreshPreview();
}

void ImageToolsUnifiedGUI::onApplyResizeBySizeClicked()
{
    if (!hasImage()) {
        return;
    }

    const int width = ui->widthValue->value();
    int height = ui->heightValue->value();

    if (keepAspectRatio) {
        const double ratio = static_cast<double>(currentImage.width()) / currentImage.height();
        height = static_cast<int>(std::round(width / ratio));
    }

    applyResize({width, height});
}

void ImageToolsUnifiedGUI::onApplyResizeByScaleClicked()
{
    if (!hasImage()) {
        return;
    }

    const double sx = ui->hScaleValue->value() / 100.0;
    const double sy = keepAspectRatio ? sx : ui->vScaleValue->value() / 100.0;
    const int width = std::max(1, static_cast<int>(std::round(currentImage.width() * sx)));
    const int height = std::max(1, static_cast<int>(std::round(currentImage.height() * sy)));

    applyResize({width, height});
}

void ImageToolsUnifiedGUI::onKeepAspectRatioChanged(Qt::CheckState state)
{
    keepAspectRatio = (state == Qt::CheckState::Checked);
}

void ImageToolsUnifiedGUI::onRotateLeftButtonClicked()
{
    if (!hasImage()) {
        return;
    }

    QTransform t;
    t.rotate(-90);
    currentImage = currentImage.transformed(t, Qt::SmoothTransformation);
    refreshPreview();
}

void ImageToolsUnifiedGUI::onRotateRightButtonClicked()
{
    if (!hasImage()) {
        return;
    }

    QTransform t;
    t.rotate(90);
    currentImage = currentImage.transformed(t, Qt::SmoothTransformation);
    refreshPreview();
}

void ImageToolsUnifiedGUI::onFlipHorizontalButtonClicked()
{
    if (!hasImage()) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    currentImage = currentImage.flipped(Qt::Horizontal);
#else
    currentImage = currentImage.mirrored(true, false);
#endif
    refreshPreview();
}

void ImageToolsUnifiedGUI::onFlipVerticalButtonClicked()
{
    if (!hasImage()) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    currentImage = currentImage.flipped(Qt::Vertical);
#else
    currentImage = currentImage.mirrored(false, true);
#endif
    refreshPreview();
}

void ImageToolsUnifiedGUI::onColorModeIndexChanged(int index)
{
    const QVariant data = ui->colorMode->itemData(index);
    if (!data.isValid()) {
        return;
    }

    transparentColorSpec = static_cast<QColor::Spec>(data.toInt());
}

void ImageToolsUnifiedGUI::onPixelSelected(const QPoint &point, const QColor &color)
{
    applyTransparencyAt(point, color);
}

void ImageToolsUnifiedGUI::onToleranceValueChanged(double tolerance)
{
    transparentTolerance = std::clamp(tolerance, 0.0, 1.0);
}

void ImageToolsUnifiedGUI::onTransparencyValueChanged(double transparency)
{
    const double clamped = std::clamp(transparency, 0.0, 1.0);
    transparentOpacity = static_cast<uint8_t>(255 * (1.0 - clamped));
}

void ImageToolsUnifiedGUI::onContiguousAreaCheckStateChanged(Qt::CheckState state)
{
    onlyContiguousArea = (state == Qt::CheckState::Checked);
}

double ImageToolsUnifiedGUI::colorDiffSquaredRgb(const QColor &a, const QColor &b)
{
    const double dr = a.redF() - b.redF();
    const double dg = a.greenF() - b.greenF();
    const double db = a.blueF() - b.blueF();
    return (dr * dr) + (dg * dg) + (db * db);
}

double ImageToolsUnifiedGUI::colorDiffSquaredHsv(const QColor &a, const QColor &b)
{
    QVector3D va;
    if (a.hsvHueF() < 0) {
        va = QVector3D(0, 0, a.valueF());
    } else {
        va = QVector3D(static_cast<float>(a.hsvSaturationF() * cos(M_TAU * a.hsvHueF())),
                       static_cast<float>(a.hsvSaturationF() * sin(M_TAU * a.hsvHueF())),
                       a.valueF());
    }

    QVector3D vb;
    if (b.hsvHueF() < 0) {
        vb = QVector3D(0, 0, b.valueF());
    } else {
        vb = QVector3D(static_cast<float>(b.hsvSaturationF() * cos(M_TAU * b.hsvHueF())),
                       static_cast<float>(b.hsvSaturationF() * sin(M_TAU * b.hsvHueF())),
                       b.valueF());
    }

    return (vb - va).lengthSquared();
}

double ImageToolsUnifiedGUI::colorDiffSquaredHsl(const QColor &a, const QColor &b)
{
    QVector3D va;
    if (a.hslHueF() < 0) {
        va = QVector3D(0, 0, a.lightnessF());
    } else {
        va = QVector3D(static_cast<float>(a.hslSaturationF() * cos(M_TAU * a.hslHueF())),
                       static_cast<float>(a.hslSaturationF() * sin(M_TAU * a.hslHueF())),
                       a.lightnessF());
    }

    QVector3D vb;
    if (b.hslHueF() < 0) {
        vb = QVector3D(0, 0, b.lightnessF());
    } else {
        vb = QVector3D(static_cast<float>(b.hslSaturationF() * cos(M_TAU * b.hslHueF())),
                       static_cast<float>(b.hslSaturationF() * sin(M_TAU * b.hslHueF())),
                       b.lightnessF());
    }

    return (vb - va).lengthSquared();
}

double ImageToolsUnifiedGUI::maxColorDiffSquared(QColor::Spec spec)
{
    switch (spec) {
    case QColor::Spec::Rgb:
        return 3.0;
    case QColor::Spec::Hsv:
    case QColor::Spec::Hsl:
        return 4.0;
    default:
        return 3.0;
    }
}

bool ImageToolsUnifiedGUI::isTransparentTarget(const QColor &source, const QColor &target,
                                               QColor::Spec spec, double tolerance)
{
    const QColor a = source.convertTo(spec);
    const QColor b = target.convertTo(spec);
    double diff = 0.0;
    switch (spec) {
    case QColor::Spec::Rgb:
        diff = colorDiffSquaredRgb(a, b);
        break;
    case QColor::Spec::Hsv:
        diff = colorDiffSquaredHsv(a, b);
        break;
    case QColor::Spec::Hsl:
        diff = colorDiffSquaredHsl(a, b);
        break;
    default:
        diff = colorDiffSquaredRgb(a, b);
        break;
    }

    return diff <= (maxColorDiffSquared(spec) * tolerance * tolerance);
}

void ImageToolsUnifiedGUI::applyTransparentByColor(const QColor &targetColor)
{
    for (int y = 0; y < currentImage.height(); y++) {
        for (int x = 0; x < currentImage.width(); x++) {
            QColor source = currentImage.pixelColor(x, y);
            if (isTransparentTarget(source, targetColor, transparentColorSpec,
                                    transparentTolerance)) {
                source.setAlpha(transparentOpacity);
                currentImage.setPixelColor(x, y, source);
            }
        }
    }
}

void ImageToolsUnifiedGUI::applyTransparentByFloodFill(const QPoint &start)
{
    if (!currentImage.valid(start)) {
        return;
    }

    const QColor targetColor = currentImage.pixelColor(start);
    QVector<bool> visited(static_cast<qsizetype>(currentImage.width()) * currentImage.height(),
                          false);
    auto index = [this](int x, int y) {
        return (static_cast<qsizetype>(y) * static_cast<qsizetype>(currentImage.width())) + x;
    };

    QQueue<QPoint> queue;
    queue.enqueue(start);
    visited[index(start.x(), start.y())] = true;

    while (!queue.isEmpty()) {
        const QPoint p = queue.dequeue();
        QColor source = currentImage.pixelColor(p);
        source.setAlpha(transparentOpacity);
        currentImage.setPixelColor(p, source);

        const std::array<QPoint, 4> neighbors = {
            QPoint(p.x() + 1, p.y()),
            QPoint(p.x() - 1, p.y()),
            QPoint(p.x(), p.y() + 1),
            QPoint(p.x(), p.y() - 1),
        };
        for (const QPoint &n : neighbors) {
            if (!currentImage.valid(n)) {
                continue;
            }

            const qsizetype i = index(n.x(), n.y());
            if (visited[i]) {
                continue;
            }
            visited[i] = true;

            if (isTransparentTarget(currentImage.pixelColor(n), targetColor, transparentColorSpec,
                                    transparentTolerance)) {
                queue.enqueue(n);
            }
        }
    }
}

void ImageToolsUnifiedGUI::applyTransparencyAt(const QPoint &point, const QColor &color)
{
    if (!hasImage()) {
        return;
    }

    ui->colorSample->setColor(color);
    if (onlyContiguousArea) {
        applyTransparentByFloodFill(point);
    } else {
        applyTransparentByColor(color);
    }

    refreshPreview();
}

void ImageToolsUnifiedGUI::onDivisionModeClicked(QAbstractButton *button)
{
    if (button == ui->useDivisionButton) {
        divisionMode = DivisionMode::DIVISION;
        ui->hDivValue->setEnabled(true);
        ui->vDivValue->setEnabled(true);
        ui->cellWidthValue->setEnabled(false);
        ui->cellHeightValue->setEnabled(false);
    } else if (button == ui->useSizeButton) {
        divisionMode = DivisionMode::CELL_SIZE;
        ui->hDivValue->setEnabled(false);
        ui->vDivValue->setEnabled(false);
        ui->cellWidthValue->setEnabled(true);
        ui->cellHeightValue->setEnabled(true);
    }
}

int ImageToolsUnifiedGUI::countByCellSize(int source, int cell, bool ignoreRemainders)
{
    if (source <= 0 || cell <= 0) {
        return 0;
    }

    const int base = source / cell;
    if (ignoreRemainders || (source % cell == 0)) {
        return base;
    }
    return base + 1;
}

QString ImageToolsUnifiedGUI::outputSuffix() const
{
    if (loadedFilePath.isEmpty()) {
        return QStringLiteral("png");
    }

    const QFileInfo info(loadedFilePath);
    const QString suffix = info.suffix();
    return suffix.isEmpty() ? QStringLiteral("png") : suffix;
}

bool ImageToolsUnifiedGUI::saveDividedImages(const QString &folderPath) const
{
    if (currentImage.isNull() || folderPath.isEmpty()) {
        return false;
    }

    const int sourceW = currentImage.width();
    const int sourceH = currentImage.height();
    if (sourceW <= 0 || sourceH <= 0) {
        return false;
    }

    const DivisionPlan plan =
        buildDivisionPlan(ui, divisionMode == DivisionMode::DIVISION, sourceW, sourceH);

    if (plan.xCount <= 0 || plan.yCount <= 0) {
        return false;
    }

    const QFileInfo info(loadedFilePath);
    const QString baseName = info.baseName().isEmpty() ? QStringLiteral("image") : info.baseName();
    const QString suffix = outputSuffix();
    const QDir dir(folderPath);

    bool result = true;
    for (int y = 0; y < plan.yCount; y++) {
        for (int x = 0; x < plan.xCount; x++) {
            const QRect rect = divisionRect(ui, plan, x, y, sourceW, sourceH);
            if (rect.isEmpty()) {
                continue;
            }

            const QString path =
                dir.filePath(baseName + QStringLiteral("_%1_%2.").arg(x).arg(y) + suffix);
            result = result && currentImage.copy(rect).save(path);
        }
    }

    return result;
}

void ImageToolsUnifiedGUI::onSaveDividedButtonClicked()
{
    if (!hasImage()) {
        return;
    }

    const QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select save folder"));
    if (folderPath.isEmpty()) {
        return;
    }

    if (!saveDividedImages(folderPath)) {
        QMessageBox::critical(this, tr("Save Failed"), tr("Failed to save divided images."));
    }
}

void ImageToolsUnifiedGUI::onSectionVisibilityChanged()
{
    const bool resizeOpen = ui->resizeSectionToggle->isChecked();
    const bool transformOpen = ui->transformSectionToggle->isChecked();
    const bool transparentOpen = ui->transparentSectionToggle->isChecked();
    const bool divisionOpen = ui->divisionSectionToggle->isChecked();

    ui->resizeSectionBody->setVisible(resizeOpen);
    ui->transformSectionBody->setVisible(transformOpen);
    ui->transparentSectionBody->setVisible(transparentOpen);
    ui->divisionSectionBody->setVisible(divisionOpen);

    retranslateSectionToggles();
}

void ImageToolsUnifiedGUI::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        event->accept();
    }
    GuiTool::changeEvent(event);
}

void ImageToolsUnifiedGUI::retranslateUi()
{
    retranslateSectionToggles();

    ui->keepAspectRatio->setText(tr("Keep aspect ratio"));
    ui->smoothScaling->setText(tr("Smooth scaling"));
    ui->applyResizeBySizeButton->setText(tr("Apply Size"));
    ui->applyResizeByScaleButton->setText(tr("Apply Scale"));

    ui->rotateLeftButton->setText(tr("Rotate Anti-clockwise"));
    ui->rotateRightButton->setText(tr("Rotate Clockwise"));
    ui->flipHorizontalButton->setText(tr("Flip Horizontal"));
    ui->flipVerticalButton->setText(tr("Flip Vertical"));

    ui->contiguousArea->setText(tr("Contiguous area only"));
    ui->useDivisionButton->setText(tr("By division count"));
    ui->useSizeButton->setText(tr("By cell size"));
    ui->ignoreRemainders->setText(tr("Ignore remainders"));
    ui->saveDividedButton->setText(tr("Save divided images"));

    const int colorModeIndex = ui->colorMode->currentIndex();
    ui->colorMode->clear();
    ui->colorMode->addItem(tr("RGB"), static_cast<int>(QColor::Spec::Rgb));
    ui->colorMode->addItem(tr("HSV"), static_cast<int>(QColor::Spec::Hsv));
    ui->colorMode->addItem(tr("HSL"), static_cast<int>(QColor::Spec::Hsl));
    ui->colorMode->setCurrentIndex(colorModeIndex);
}

void ImageToolsUnifiedGUI::retranslateSectionToggles()
{
    const bool resizeOpen = ui->resizeSectionToggle->isChecked();
    const bool transformOpen = ui->transformSectionToggle->isChecked();
    const bool transparentOpen = ui->transparentSectionToggle->isChecked();
    const bool divisionOpen = ui->divisionSectionToggle->isChecked();

    ui->resizeSectionToggle->setText(
        (resizeOpen ? QStringLiteral("\u25BC ") : QStringLiteral("\u25B6 ")) + tr("Resize"));
    ui->transformSectionToggle->setText(
        (transformOpen ? QStringLiteral("\u25BC ") : QStringLiteral("\u25B6 ")) +
        tr("Rotate / Flip"));
    ui->transparentSectionToggle->setText(
        (transparentOpen ? QStringLiteral("\u25BC ") : QStringLiteral("\u25B6 ")) +
        tr("Transparent (click image)"));
    ui->divisionSectionToggle->setText(
        (divisionOpen ? QStringLiteral("\u25BC ") : QStringLiteral("\u25B6 ")) + tr("Division"));
}
