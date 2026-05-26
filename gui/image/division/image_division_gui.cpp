#include "image_division_gui.h"

#include "core/exception/invalid_state_exception.h"
#include "core/image/division/image_division.h"
#include "gui/image/basic/control.h"
#include "gui/image/division/image_view_for_image_division.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpacerItem>
#include <QSpinBox>
#include <QVBoxLayout>

#include <array>

#include <core/exception/invalid_argument_exception.h>

namespace {
void buildSizeRow(Ui::ImageDivisionGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *row = new QHBoxLayout();
    row->setSpacing(4);

    auto *label = new QLabel(ImageDivisionGUI::tr("Image size:"), parent);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    row->addWidget(label);

    ui->sizeLabel = new QLabel(ImageDivisionGUI::tr("0 x 0"), parent);
    ui->sizeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    row->addWidget(ui->sizeLabel);

    layout->addLayout(row);
}

void buildDivisionGrid(Ui::ImageDivisionGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *grid = new QGridLayout();

    auto *hLabel = new QLabel(ImageDivisionGUI::tr("Horizontal:"), parent);
    hLabel->setLayoutDirection(Qt::LeftToRight);
    hLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    grid->addWidget(hLabel, 0, 0);

    ui->hDivValue = new QSpinBox(parent);
    ui->hDivValue->setMinimum(1);
    grid->addWidget(ui->hDivValue, 0, 1);

    auto *vLabel = new QLabel(ImageDivisionGUI::tr("Vertical:"), parent);
    vLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    grid->addWidget(vLabel, 1, 0);

    ui->vDivValue = new QSpinBox(parent);
    ui->vDivValue->setMinimum(1);
    grid->addWidget(ui->vDivValue, 1, 1);

    layout->addLayout(grid);
}

void buildSizeGrid(Ui::ImageDivisionGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *grid = new QGridLayout();

    auto *wLabel = new QLabel(ImageDivisionGUI::tr("Width:"), parent);
    wLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    grid->addWidget(wLabel, 0, 0);

    ui->widthValue = new QSpinBox(parent);
    ui->widthValue->setEnabled(false);
    ui->widthValue->setSuffix("px");
    grid->addWidget(ui->widthValue, 0, 1);

    auto *hLabel = new QLabel(ImageDivisionGUI::tr("Height:"), parent);
    hLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    grid->addWidget(hLabel, 1, 0);

    ui->heightValue = new QSpinBox(parent);
    ui->heightValue->setEnabled(false);
    ui->heightValue->setSuffix("px");
    grid->addWidget(ui->heightValue, 1, 1);

    layout->addLayout(grid);
}

void buildUiArea(Ui::ImageDivisionGUI *ui, QWidget *parent, QHBoxLayout *rootLayout)
{
    auto *uiArea = new QFrame(parent);
    uiArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    uiArea->setMinimumWidth(240);
    uiArea->setFrameShape(QFrame::StyledPanel);

    auto *layout = new QVBoxLayout(uiArea);
    layout->setContentsMargins(12, 12, 12, 12);

    buildSizeRow(ui, uiArea, layout);

    ui->useDivisionButton = new QRadioButton(ImageDivisionGUI::tr("Division"), uiArea);
    ui->useDivisionButton->setChecked(true);
    layout->addWidget(ui->useDivisionButton);

    buildDivisionGrid(ui, uiArea, layout);

    ui->useSizeButton = new QRadioButton(ImageDivisionGUI::tr("Size"), uiArea);
    layout->addWidget(ui->useSizeButton);

    buildSizeGrid(ui, uiArea, layout);

    ui->divisionModeButtonGroup = new QButtonGroup(parent);
    ui->divisionModeButtonGroup->addButton(ui->useDivisionButton);
    ui->divisionModeButtonGroup->addButton(ui->useSizeButton);

    ui->ignoreRemainders = new QCheckBox(ImageDivisionGUI::tr("Ignore remainders"), uiArea);
    ui->ignoreRemainders->setChecked(true);
    layout->addWidget(ui->ignoreRemainders);

    layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    ui->control = new BasicImageViewControl(uiArea);
    layout->addWidget(ui->control);

    rootLayout->addWidget(uiArea);
}
} // namespace

ImageDivisionGUI::ImageDivisionGUI(ImageDivisionInterface *imageDivision, QWidget *parent)
    : GuiTool(parent), imageDivision(imageDivision), ui(new Ui::ImageDivisionGUI)
{
    resize(400, 300);

    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    ui->imageView = new ImageViewForImageDivision(this);
    rootLayout->addWidget(ui->imageView);

    buildUiArea(ui, this, rootLayout);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageDivision->parent() == nullptr) {
        imageDivision->setParent(this);
    }

    ui->control->saveFileDialogType = BasicImageViewControl::DialogType::SELECT_FOLDER;
    connect(ui->imageView, &BasicImageView::loadFileSelected, ui->control,
            &BasicImageViewControl::loadFileSelected);
    connect(ui->control, &BasicImageViewControl::loadFileSelected, this,
            &ImageDivisionGUI::onLoadImageSelected);
    connect(ui->control, &BasicImageViewControl::saveFileSelected, this,
            &ImageDivisionGUI::onSaveLocationSelected);
    connect(ui->control, &BasicImageViewControl::resetButtonClicked, this,
            &ImageDivisionGUI::onResetButtonClicked);

    connect(ui->divisionModeButtonGroup, &QButtonGroup::buttonClicked, this,
            &ImageDivisionGUI::onDivisionModeClicked);

    connect(ui->ignoreRemainders, &QCheckBox::toggled, this,
            &ImageDivisionGUI::onDiscardRemainderToggled);

    connect(ui->hDivValue, &QSpinBox::valueChanged, this,
            &ImageDivisionGUI::onHorizontalDivisionValueChanged);
    connect(ui->vDivValue, &QSpinBox::valueChanged, this,
            &ImageDivisionGUI::onVerticalDivisionValueChanged);
    connect(ui->widthValue, &QSpinBox::valueChanged, this, &ImageDivisionGUI::onWidthValueChanged);
    connect(ui->heightValue, &QSpinBox::valueChanged, this,
            &ImageDivisionGUI::onHeightValueChanged);
}

ImageDivisionGUI::~ImageDivisionGUI()
{
    delete ui;
}

void ImageDivisionGUI::onLoadImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    imageDivision->reset();
    imageDivision->load(path);
    imageDivision->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageDivision->current()), true);
    const auto size = imageDivision->current().size();
    ui->sizeLabel->setText(tr("%1 x %2").arg(size.width()).arg(size.height()));

    // NOTE: 最大値・最小値を設定する
    const std::array<const QSignalBlocker, 4> blockers = {{
        QSignalBlocker(ui->hDivValue),
        QSignalBlocker(ui->vDivValue),
        QSignalBlocker(ui->widthValue),
        QSignalBlocker(ui->heightValue),
    }};

    ui->hDivValue->setMaximum(size.width());
    ui->vDivValue->setMaximum(size.height());
    ui->widthValue->setMaximum(size.width());
    ui->widthValue->setMinimum(1);
    ui->heightValue->setMaximum(size.height());
    ui->heightValue->setMinimum(1);

    updateUI();

    // TODO: load()の結果に応じて何かメッセージを出す
}

void ImageDivisionGUI::onSaveLocationSelected(const QString &path)
{
    qDebug() << "path:" << path;

    if (!imageDivision->overwriteSave(path)) {
        QMessageBox::critical(this, tr("Save Failed"), tr("Failed to save the image."));
    }
}

void ImageDivisionGUI::onResetButtonClicked()
{
    imageDivision->reset();
    updateUI();
}

void ImageDivisionGUI::onDivisionModeClicked(QAbstractButton *button)
{
    if (button == ui->useDivisionButton) {
        ui->widthValue->setDisabled(true);
        ui->heightValue->setDisabled(true);
        ui->hDivValue->setEnabled(true);
        ui->vDivValue->setEnabled(true);

        imageDivision->setHorizontalDivision(ui->hDivValue->value());
        imageDivision->setVerticalDivision(ui->vDivValue->value());
    } else if (button == ui->useSizeButton) {
        ui->widthValue->setEnabled(true);
        ui->heightValue->setEnabled(true);
        ui->hDivValue->setDisabled(true);
        ui->vDivValue->setDisabled(true);

        imageDivision->setCellWidth(ui->widthValue->value());
        imageDivision->setCellHeight(ui->heightValue->value());
    } else {
        throw InvalidArgumentException<void *>("Unknown button is clicked");
    }

    updateUI();
}

void ImageDivisionGUI::onHorizontalDivisionValueChanged(int hDiv)
{
    if (!ui->useDivisionButton->isEnabled()) {
        throw InvalidStateException("useDivisionButton expected to be checked");
    }

    imageDivision->setHorizontalDivision(hDiv);
    updateUI();
}

void ImageDivisionGUI::onVerticalDivisionValueChanged(int vDiv)
{
    if (!ui->useDivisionButton->isEnabled()) {
        throw InvalidStateException("useDivisionButton expected to be checked");
    }

    imageDivision->setVerticalDivision(vDiv);
    updateUI();
}

void ImageDivisionGUI::onWidthValueChanged(int width)
{
    if (!ui->useSizeButton->isEnabled()) {
        throw InvalidStateException("useSizeButton expected to be checked");
    }

    imageDivision->setCellWidth(width);
    updateUI();
}

void ImageDivisionGUI::onHeightValueChanged(int height)
{
    if (!ui->useSizeButton->isEnabled()) {
        throw InvalidStateException("useSizeButton expected to be checked");
    }

    imageDivision->setCellHeight(height);
    updateUI();
}

void ImageDivisionGUI::updateUI()
{
    const auto cellSize = imageDivision->computedCellSize();
    const std::array<const QSignalBlocker, 4> blockers = {{
        QSignalBlocker(ui->hDivValue),
        QSignalBlocker(ui->vDivValue),
        QSignalBlocker(ui->widthValue),
        QSignalBlocker(ui->heightValue),
    }};
    ui->hDivValue->setValue(static_cast<int>(imageDivision->numberOfHorizontalDivision()));
    ui->vDivValue->setValue(static_cast<int>(imageDivision->numberOfVerticalDivision()));
    ui->widthValue->setValue(static_cast<int>(cellSize.width()));
    ui->heightValue->setValue(static_cast<int>(cellSize.height()));

    ui->imageView->setGridSize(cellSize);
}

void ImageDivisionGUI::onDiscardRemainderToggled(bool checked)
{
    imageDivision->discardRemainders = checked;
}
