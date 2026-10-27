#include "image_tools_unified_gui.h"

#include "features/framework/gui/design_system.h"
#include "features/framework/gui/icon_utils.h"
#include "features/image/core/image_processing_service.h"
#include "features/image/gui/basic/control.h"
#include "features/image/gui/transparent/color_sample.h"
#include "features/image/gui/transparent/image_view_for_image_transparent.h"

#include <QAbstractButton>
#include <QAbstractSpinBox>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

namespace {
QSpinBox *buildPixelSpinBox(QWidget *parent)
{
    auto *spin = new QSpinBox(parent);
    spin->setSuffix("px");
    spin->setMinimum(1);
    spin->setMaximum(16777215);
    spin->setSingleStep(1);
    spin->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    spin->setDisplayIntegerBase(10);
    DevTools::Ui::configureFormField(spin);
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
    DevTools::Ui::configureFormField(spin);
    return spin;
}

QPushButton *buildActionButton(QWidget *parent, const QString &text, const QString &iconName)
{
    auto *button = new QPushButton(IconUtils::themedIcon(iconName), text, parent);
    DevTools::Ui::configurePrimaryButton(button);
    return button;
}

void buildResizeSection(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *frame = new QFrame(parent);
    ui->resizeSection = frame;
    DevTools::Ui::configureSection(frame);
    auto *box = new QVBoxLayout(frame);
    DevTools::Ui::applyPanelLayout(box);
    ui->resizeSectionToggle = new QCheckBox(ImageToolsUnifiedGUI::tr("Resize"), frame);
    ui->resizeSectionToggle->setChecked(false);
    box->addWidget(ui->resizeSectionToggle);

    ui->resizeSectionBody = new QWidget(frame);
    auto *bodyLayout = new QVBoxLayout(ui->resizeSectionBody);
    DevTools::Ui::applyInlineLayout(bodyLayout);

    auto *form = new QFormLayout();
    DevTools::Ui::configureInlineFormLayout(form);
    ui->widthValue = buildPixelSpinBox(ui->resizeSectionBody);
    ui->heightValue = buildPixelSpinBox(ui->resizeSectionBody);
    ui->hScaleValue = buildScaleSpinBox(ui->resizeSectionBody);
    ui->vScaleValue = buildScaleSpinBox(ui->resizeSectionBody);
    ui->widthLabel = new QLabel(ImageToolsUnifiedGUI::tr("Width:"), ui->resizeSectionBody);
    ui->heightLabel = new QLabel(ImageToolsUnifiedGUI::tr("Height:"), ui->resizeSectionBody);
    ui->horizontalScaleLabel =
        new QLabel(ImageToolsUnifiedGUI::tr("Horizontal Scale:"), ui->resizeSectionBody);
    ui->verticalScaleLabel =
        new QLabel(ImageToolsUnifiedGUI::tr("Vertical Scale:"), ui->resizeSectionBody);
    form->addRow(ui->widthLabel, ui->widthValue);
    form->addRow(ui->heightLabel, ui->heightValue);
    form->addRow(ui->horizontalScaleLabel, ui->hScaleValue);
    form->addRow(ui->verticalScaleLabel, ui->vScaleValue);
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
    DevTools::Ui::configurePrimaryButton(ui->applyResizeBySizeButton);
    DevTools::Ui::configurePrimaryButton(ui->applyResizeByScaleButton);
    bodyLayout->addWidget(ui->applyResizeBySizeButton);
    bodyLayout->addWidget(ui->applyResizeByScaleButton);

    box->addWidget(ui->resizeSectionBody);

    layout->addWidget(frame);
}

void buildTransformSection(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *frame = new QFrame(parent);
    ui->transformSection = frame;
    DevTools::Ui::configureSection(frame);
    auto *box = new QVBoxLayout(frame);
    DevTools::Ui::applyPanelLayout(box);
    ui->transformSectionToggle = new QCheckBox(ImageToolsUnifiedGUI::tr("Rotate / Flip"), frame);
    ui->transformSectionToggle->setChecked(false);
    box->addWidget(ui->transformSectionToggle);

    ui->transformSectionBody = new QWidget(frame);
    auto *bodyLayout = new QVBoxLayout(ui->transformSectionBody);
    DevTools::Ui::applyInlineLayout(bodyLayout);

    ui->rotateLeftButton = buildActionButton(
        ui->transformSectionBody, ImageToolsUnifiedGUI::tr("Rotate Anti-clockwise"), "rotate_left");
    ui->rotateRightButton = buildActionButton(
        ui->transformSectionBody, ImageToolsUnifiedGUI::tr("Rotate Clockwise"), "rotate_right");
    ui->flipHorizontalButton = buildActionButton(
        ui->transformSectionBody, ImageToolsUnifiedGUI::tr("Flip Horizontal"), "swap_horiz");
    ui->flipVerticalButton = buildActionButton(
        ui->transformSectionBody, ImageToolsUnifiedGUI::tr("Flip Vertical"), "swap_vert");

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
    DevTools::Ui::configureSection(frame);
    auto *box = new QVBoxLayout(frame);
    DevTools::Ui::applyPanelLayout(box);
    ui->transparentSectionToggle =
        new QCheckBox(ImageToolsUnifiedGUI::tr("Transparent (click image)"), frame);
    ui->transparentSectionToggle->setChecked(false);
    box->addWidget(ui->transparentSectionToggle);

    ui->transparentSectionBody = new QWidget(frame);
    auto *bodyLayout = new QVBoxLayout(ui->transparentSectionBody);
    DevTools::Ui::applyInlineLayout(bodyLayout);

    auto *form = new QFormLayout();
    DevTools::Ui::configureInlineFormLayout(form);
    ui->colorMode = new QComboBox(ui->transparentSectionBody);
    DevTools::Ui::configureComboBox(ui->colorMode);
    ui->colorMode->addItem(QStringLiteral("RGB"), static_cast<int>(QColor::Spec::Rgb));
    ui->colorMode->addItem(QStringLiteral("HSL"), static_cast<int>(QColor::Spec::Hsl));
    ui->colorMode->addItem(QStringLiteral("HSV"), static_cast<int>(QColor::Spec::Hsv));
    ui->colorModeLabel =
        new QLabel(ImageToolsUnifiedGUI::tr("Color mode:"), ui->transparentSectionBody);
    form->addRow(ui->colorModeLabel, ui->colorMode);

    ui->colorSample = new ColorSample(ui->transparentSectionBody);
    ui->colorLabel = new QLabel(ImageToolsUnifiedGUI::tr("Color:"), ui->transparentSectionBody);
    form->addRow(ui->colorLabel, ui->colorSample);

    ui->toleranceValue = new QDoubleSpinBox(ui->transparentSectionBody);
    DevTools::Ui::configureFormField(ui->toleranceValue);
    ui->toleranceValue->setMinimum(0.0);
    ui->toleranceValue->setMaximum(1.0);
    ui->toleranceValue->setSingleStep(0.05);
    ui->toleranceValue->setValue(0.1);
    ui->toleranceLabel =
        new QLabel(ImageToolsUnifiedGUI::tr("Tolerance:"), ui->transparentSectionBody);
    form->addRow(ui->toleranceLabel, ui->toleranceValue);

    ui->transparencyValue = new QDoubleSpinBox(ui->transparentSectionBody);
    DevTools::Ui::configureFormField(ui->transparencyValue);
    ui->transparencyValue->setMinimum(0.0);
    ui->transparencyValue->setMaximum(1.0);
    ui->transparencyValue->setSingleStep(0.05);
    ui->transparencyValue->setValue(1.0);
    ui->transparencyLabel =
        new QLabel(ImageToolsUnifiedGUI::tr("Transparency:"), ui->transparentSectionBody);
    form->addRow(ui->transparencyLabel, ui->transparencyValue);
    bodyLayout->addLayout(form);

    ui->contiguousArea =
        new QCheckBox(ImageToolsUnifiedGUI::tr("Only contiguous area"), ui->transparentSectionBody);
    ui->contiguousArea->setChecked(true);
    bodyLayout->addWidget(ui->contiguousArea);

    box->addWidget(ui->transparentSectionBody);

    layout->addWidget(frame);
}

void buildDivisionValueGrid(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *grid = new QGridLayout();
    DevTools::Ui::applyInlineLayout(grid);
    DevTools::Ui::configureCaptionValueGrid(grid);
    ui->hDivValue = new QSpinBox(parent);
    DevTools::Ui::configureFormField(ui->hDivValue);
    ui->hDivValue->setMinimum(1);
    ui->vDivValue = new QSpinBox(parent);
    DevTools::Ui::configureFormField(ui->vDivValue);
    ui->vDivValue->setMinimum(1);
    ui->cellWidthValue = buildPixelSpinBox(parent);
    ui->cellHeightValue = buildPixelSpinBox(parent);
    ui->cellWidthValue->setEnabled(false);
    ui->cellHeightValue->setEnabled(false);

    ui->horizontalDivisionLabel =
        new QLabel(ImageToolsUnifiedGUI::tr("Horizontal division:"), parent);
    grid->addWidget(ui->horizontalDivisionLabel, 0, 0);
    grid->addWidget(ui->hDivValue, 0, 1);
    ui->verticalDivisionLabel = new QLabel(ImageToolsUnifiedGUI::tr("Vertical division:"), parent);
    grid->addWidget(ui->verticalDivisionLabel, 1, 0);
    grid->addWidget(ui->vDivValue, 1, 1);
    ui->cellWidthLabel = new QLabel(ImageToolsUnifiedGUI::tr("Cell width:"), parent);
    grid->addWidget(ui->cellWidthLabel, 2, 0);
    grid->addWidget(ui->cellWidthValue, 2, 1);
    ui->cellHeightLabel = new QLabel(ImageToolsUnifiedGUI::tr("Cell height:"), parent);
    grid->addWidget(ui->cellHeightLabel, 3, 0);
    grid->addWidget(ui->cellHeightValue, 3, 1);
    layout->addLayout(grid);
}

void buildDivisionSection(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *frame = new QFrame(parent);
    ui->divisionSection = frame;
    DevTools::Ui::configureSection(frame);
    auto *box = new QVBoxLayout(frame);
    DevTools::Ui::applyPanelLayout(box);
    ui->divisionSectionToggle = new QCheckBox(ImageToolsUnifiedGUI::tr("Division"), frame);
    ui->divisionSectionToggle->setChecked(false);
    box->addWidget(ui->divisionSectionToggle);

    ui->divisionSectionBody = new QWidget(frame);
    auto *bodyLayout = new QVBoxLayout(ui->divisionSectionBody);
    DevTools::Ui::applyInlineLayout(bodyLayout);

    auto *sizeRow = new QHBoxLayout();
    DevTools::Ui::applyInlineLayout(sizeRow);
    ui->imageSizeLabel =
        new QLabel(ImageToolsUnifiedGUI::tr("Image size:"), ui->divisionSectionBody);
    sizeRow->addWidget(ui->imageSizeLabel);
    ui->sizeLabel = new QLabel(ImageToolsUnifiedGUI::tr("0 x 0"), ui->divisionSectionBody);
    sizeRow->addWidget(ui->sizeLabel);
    bodyLayout->addLayout(sizeRow);

    ui->useDivisionButton =
        new QRadioButton(ImageToolsUnifiedGUI::tr("Division"), ui->divisionSectionBody);
    ui->useDivisionButton->setChecked(true);
    ui->useSizeButton = new QRadioButton(ImageToolsUnifiedGUI::tr("Size"), ui->divisionSectionBody);
    bodyLayout->addWidget(ui->useDivisionButton);
    bodyLayout->addWidget(ui->useSizeButton);

    buildDivisionValueGrid(ui, ui->divisionSectionBody, bodyLayout);

    ui->divisionModeButtonGroup = new QButtonGroup(parent);
    ui->divisionModeButtonGroup->addButton(ui->useDivisionButton);
    ui->divisionModeButtonGroup->addButton(ui->useSizeButton);

    ui->ignoreRemainders =
        new QCheckBox(ImageToolsUnifiedGUI::tr("Ignore remainders"), ui->divisionSectionBody);
    ui->ignoreRemainders->setChecked(true);
    bodyLayout->addWidget(ui->ignoreRemainders);

    ui->saveDividedButton =
        new QPushButton(ImageToolsUnifiedGUI::tr("Save Divided Images"), ui->divisionSectionBody);
    DevTools::Ui::configurePrimaryButton(ui->saveDividedButton);
    bodyLayout->addWidget(ui->saveDividedButton);

    box->addWidget(ui->divisionSectionBody);

    layout->addWidget(frame);
}

void buildUiArea(Ui::ImageToolsUnifiedGUI *ui, QWidget *parent, QHBoxLayout *rootLayout)
{
    auto *toolPane = DevTools::Ui::createPane(ImageToolsUnifiedGUI::tr("Operation Panel"), parent);
    ui->toolPane = toolPane;
    auto *toolPaneLayout = new QVBoxLayout(toolPane);
    DevTools::Ui::applyPanelLayout(toolPaneLayout);

    ui->toolScrollArea = new QScrollArea(toolPane);
    ui->toolScrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->toolScrollArea->setMinimumWidth(DevTools::Ui::Metrics::TOOL_PANEL_WIDTH);
    ui->toolScrollArea->setWidgetResizable(true);
    ui->toolScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    DevTools::Ui::configureScrollView(ui->toolScrollArea);
    DevTools::Ui::configurePaneSurface(ui->toolScrollArea);

    auto *uiArea = new QWidget(ui->toolScrollArea);
    auto *layout = new QVBoxLayout(uiArea);
    DevTools::Ui::applyPanelLayout(layout);

    buildResizeSection(ui, uiArea, layout);
    buildTransformSection(ui, uiArea, layout);
    buildTransparentSection(ui, uiArea, layout);
    buildDivisionSection(ui, uiArea, layout);

    layout->addStretch();

    ui->toolScrollArea->setWidget(uiArea);
    toolPaneLayout->addWidget(ui->toolScrollArea, 1);

    ui->control = new BasicImageViewControl(toolPane);
    toolPaneLayout->addWidget(ui->control);
    rootLayout->addWidget(toolPane, DevTools::Ui::Metrics::SIDE_PANEL_STRETCH);
}

} // namespace

ImageToolsUnifiedGUI::ImageToolsUnifiedGUI(QWidget *parent)
    : GuiTool(parent), ui(new Ui::ImageToolsUnifiedGUI)
{
    auto *rootLayout = new QHBoxLayout(this);
    DevTools::Ui::applyPageLayout(rootLayout);

    auto *imagePane = DevTools::Ui::createPane(ImageToolsUnifiedGUI::tr("Image"), this);
    ui->imagePane = imagePane;
    auto *imagePaneLayout = new QVBoxLayout(imagePane);
    DevTools::Ui::applyPanelLayout(imagePaneLayout);

    ui->imageView = new ImageViewForImageTransparent(imagePane);
    DevTools::Ui::configureExpandingSurface(ui->imageView);
    DevTools::Ui::configurePaneSurface(ui->imageView);
    imagePaneLayout->addWidget(ui->imageView);
    rootLayout->addWidget(imagePane, DevTools::Ui::Metrics::MAIN_PANEL_STRETCH);

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

void ImageToolsUnifiedGUI::retranslateUi()
{
    setWindowTitle(tr("Image Tools"));
    ui->imagePane->setTitle(tr("Image"));
    ui->toolPane->setTitle(tr("Operation Panel"));

    ui->widthLabel->setText(tr("Width:"));
    ui->heightLabel->setText(tr("Height:"));
    ui->horizontalScaleLabel->setText(tr("Horizontal Scale:"));
    ui->verticalScaleLabel->setText(tr("Vertical Scale:"));
    ui->keepAspectRatio->setText(tr("Keep aspect ratio"));
    ui->smoothScaling->setText(tr("Smooth scaling"));
    ui->applyResizeBySizeButton->setText(tr("Apply Size"));
    ui->applyResizeByScaleButton->setText(tr("Apply Scale"));

    ui->rotateLeftButton->setText(tr("Rotate Anti-clockwise"));
    ui->rotateRightButton->setText(tr("Rotate Clockwise"));
    ui->flipHorizontalButton->setText(tr("Flip Horizontal"));
    ui->flipVerticalButton->setText(tr("Flip Vertical"));

    ui->colorModeLabel->setText(tr("Color mode:"));
    ui->colorLabel->setText(tr("Color:"));
    ui->toleranceLabel->setText(tr("Tolerance:"));
    ui->transparencyLabel->setText(tr("Transparency:"));
    ui->contiguousArea->setText(tr("Only contiguous area"));

    ui->imageSizeLabel->setText(tr("Image size:"));
    ui->useDivisionButton->setText(tr("Division"));
    ui->useSizeButton->setText(tr("Size"));
    ui->horizontalDivisionLabel->setText(tr("Horizontal division:"));
    ui->verticalDivisionLabel->setText(tr("Vertical division:"));
    ui->cellWidthLabel->setText(tr("Cell width:"));
    ui->cellHeightLabel->setText(tr("Cell height:"));
    ui->ignoreRemainders->setText(tr("Ignore remainders"));
    ui->saveDividedButton->setText(tr("Save Divided Images"));

    refreshSizeInputs();
    onSectionVisibilityChanged();
}

void ImageToolsUnifiedGUI::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        event->accept();
    } else {
        GuiTool::changeEvent(event);
    }
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
    const QImage loaded = ImageProcessingService::load(path);
    if (loaded.isNull()) {
        QMessageBox::critical(this, tr("Load Failed"), tr("Failed to load the image."));
        return;
    }

    loadedFilePath = path;
    originalImage = loaded;
    currentImage = originalImage;

    refreshPreview(true);
}

void ImageToolsUnifiedGUI::onSaveImageSelected(const QString &path)
{
    if (!hasImage()) {
        return;
    }

    if (!ImageProcessingService::save(currentImage, path)) {
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

void ImageToolsUnifiedGUI::onApplyResizeBySizeClicked()
{
    if (!hasImage()) {
        return;
    }

    currentImage = ImageProcessingService::resizeBySize(currentImage, ui->widthValue->value(),
                                                        ui->heightValue->value(), keepAspectRatio,
                                                        ui->smoothScaling->isChecked());
    refreshPreview();
}

void ImageToolsUnifiedGUI::onApplyResizeByScaleClicked()
{
    if (!hasImage()) {
        return;
    }

    currentImage = ImageProcessingService::resizeByScale(currentImage, ui->hScaleValue->value(),
                                                         ui->vScaleValue->value(), keepAspectRatio,
                                                         ui->smoothScaling->isChecked());
    refreshPreview();
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

    currentImage = ImageProcessingService::rotateLeft(currentImage);
    refreshPreview();
}

void ImageToolsUnifiedGUI::onRotateRightButtonClicked()
{
    if (!hasImage()) {
        return;
    }

    currentImage = ImageProcessingService::rotateRight(currentImage);
    refreshPreview();
}

void ImageToolsUnifiedGUI::onFlipHorizontalButtonClicked()
{
    if (!hasImage()) {
        return;
    }

    currentImage = ImageProcessingService::flip(currentImage, ImageFlipDirection::Horizontal);
    refreshPreview();
}

void ImageToolsUnifiedGUI::onFlipVerticalButtonClicked()
{
    if (!hasImage()) {
        return;
    }

    currentImage = ImageProcessingService::flip(currentImage, ImageFlipDirection::Vertical);
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
    transparentOpacity = ImageProcessingService::alphaForTransparency(transparency);
}

void ImageToolsUnifiedGUI::onContiguousAreaCheckStateChanged(Qt::CheckState state)
{
    onlyContiguousArea = (state == Qt::CheckState::Checked);
}

void ImageToolsUnifiedGUI::applyTransparencyAt(const QPoint &point, const QColor &color)
{
    if (!hasImage()) {
        return;
    }

    ui->colorSample->setColor(color);
    currentImage = ImageProcessingService::makeTransparent(
        currentImage, point, color, transparentColorSpec, transparentTolerance, transparentOpacity,
        onlyContiguousArea);

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

void ImageToolsUnifiedGUI::onSaveDividedButtonClicked()
{
    if (!hasImage()) {
        return;
    }

    const QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select save folder"));
    if (folderPath.isEmpty()) {
        return;
    }

    ImageSplitOptions options;
    options.mode = divisionMode == DivisionMode::DIVISION ? ImageSplitMode::DivisionCount
                                                          : ImageSplitMode::CellSize;
    options.horizontalDivisions = ui->hDivValue->value();
    options.verticalDivisions = ui->vDivValue->value();
    options.cellWidth = ui->cellWidthValue->value();
    options.cellHeight = ui->cellHeightValue->value();
    options.ignoreRemainders = ui->ignoreRemainders->isChecked();
    if (!ImageProcessingService::saveDividedImages(currentImage, loadedFilePath, folderPath,
                                                   options)) {
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

    ui->resizeSectionToggle->setText((resizeOpen ? QStringLiteral("▼ ") : QStringLiteral("▶ ")) +
                                     tr("Resize"));
    ui->transformSectionToggle->setText(
        (transformOpen ? QStringLiteral("▼ ") : QStringLiteral("▶ ")) + tr("Rotate / Flip"));
    ui->transparentSectionToggle->setText(
        (transparentOpen ? QStringLiteral("▼ ") : QStringLiteral("▶ ")) +
        tr("Transparent (click image)"));
    ui->divisionSectionToggle->setText(
        (divisionOpen ? QStringLiteral("▼ ") : QStringLiteral("▶ ")) + tr("Division"));
}
