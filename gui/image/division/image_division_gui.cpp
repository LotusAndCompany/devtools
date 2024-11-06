#include "image_division_gui.h"
#include "ui_image_division_gui.h"

#include <QButtonGroup>
#include <QSignalBlocker>
#include "core/exception/invalid_state_exception.h"
#include "core/image/division/image_division.h"
#include <core/exception/invalid_argument_exception.h>

ImageDivisionGUI::ImageDivisionGUI(ImageDivisionInterface *imageDivision, QWidget *parent)
    : QWidget(parent)
    , imageDivision(imageDivision)
    , ui(new Ui::ImageDivisionGUI)
{
    ui->setupUi(this);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageDivision->parent() == nullptr)
        imageDivision->setParent(this);

    ui->control->saveFileDailogType = BasicImageViewControl::DialogType::SELECT_FOLDER;
    connect(ui->control,
            &BasicImageViewControl::loadFileSelected,
            this,
            &ImageDivisionGUI::onLoadImageSelected);
    connect(ui->control,
            &BasicImageViewControl::saveFileSelected,
            this,
            &ImageDivisionGUI::onSaveLocationSelected);
    connect(ui->control,
            &BasicImageViewControl::resetButtonClicked,
            this,
            &ImageDivisionGUI::onResetButtonClicked);

    connect(ui->divisionModeButtonGroup,
            &QButtonGroup::buttonClicked,
            this,
            &ImageDivisionGUI::onDivisionModeClicked);

    connect(ui->ignoreRemainders,
            &QCheckBox::toggled,
            this,
            &ImageDivisionGUI::onDiscardRemainderToggled);

    connect(ui->hDivValue,
            &QSpinBox::valueChanged,
            this,
            &ImageDivisionGUI::onHorizontalDivisionValueChanged);
    connect(ui->vDivValue,
            &QSpinBox::valueChanged,
            this,
            &ImageDivisionGUI::onVerticalDivisionValueChanged);
    connect(ui->widthValue, &QSpinBox::valueChanged, this, &ImageDivisionGUI::onWidthValueChanged);
    connect(ui->heightValue, &QSpinBox::valueChanged, this, &ImageDivisionGUI::onHeightValueChanged);
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
    QSignalBlocker blockers[] = {
        QSignalBlocker(ui->hDivValue),
        QSignalBlocker(ui->vDivValue),
        QSignalBlocker(ui->widthValue),
        QSignalBlocker(ui->heightValue),
    };

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

    imageDivision->overwriteSave(path);

    // TODO: save()の結果に応じて何かメッセージを出す
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
    if (!ui->useDivisionButton->isEnabled())
        throw InvalidStateException("useDivisionButton expected to be checked");

    imageDivision->setHorizontalDivision(hDiv);
    updateUI();
}

void ImageDivisionGUI::onVerticalDivisionValueChanged(int vDiv)
{
    if (!ui->useDivisionButton->isEnabled())
        throw InvalidStateException("useDivisionButton expected to be checked");

    imageDivision->setVerticalDivision(vDiv);
    updateUI();
}

void ImageDivisionGUI::onWidthValueChanged(int width)
{
    if (!ui->useSizeButton->isEnabled())
        throw InvalidStateException("useSizeButton expected to be checked");

    imageDivision->setCellWidth(width);
    updateUI();
}

void ImageDivisionGUI::onHeightValueChanged(int height)
{
    if (!ui->useSizeButton->isEnabled())
        throw InvalidStateException("useSizeButton expected to be checked");

    imageDivision->setCellHeight(height);
    updateUI();
}

void ImageDivisionGUI::updateUI()
{
    const auto cellSize = imageDivision->computedCellSize();
    QSignalBlocker blockers[] = {
        QSignalBlocker(ui->hDivValue),
        QSignalBlocker(ui->vDivValue),
        QSignalBlocker(ui->widthValue),
        QSignalBlocker(ui->heightValue),
    };
    ui->hDivValue->setValue(imageDivision->numberOfHorizontalDivision());
    ui->vDivValue->setValue(imageDivision->numberOfVerticalDivision());
    ui->widthValue->setValue(cellSize.width());
    ui->heightValue->setValue(cellSize.height());

    ui->imageView->setGridSize(cellSize);
}

void ImageDivisionGUI::onDiscardRemainderToggled(bool checked)
{
    imageDivision->discardRemainders = checked;
}
