#include "image_rotation_gui.h"
#include "ui_image_rotation_gui.h"

#include "core/image/rotation/image_rotation.h"

#include <QMessageBox>

ImageRotationGUI::ImageRotationGUI(ImageRotationInterface *imageRotation, QWidget *parent)
    : GuiTool(parent)
    , imageRotation(imageRotation)
    , ui(new Ui::ImageRotationGUI)
{
    ui->setupUi(this);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageRotation->parent() == nullptr)
        imageRotation->setParent(this);

    connect(ui->basicImageViewControl,
            &BasicImageViewControl::loadFileSelected,
            this,
            &ImageRotationGUI::onLoadImageSelected);
    connect(ui->basicImageViewControl,
            &BasicImageViewControl::saveFileSelected,
            this,
            &ImageRotationGUI::onSaveImageSelected);
    connect(ui->basicImageViewControl,
            &BasicImageViewControl::resetButtonClicked,
            this,
            &ImageRotationGUI::onResetButtonClicked);

    connect(ui->rotateLeftButton,
            &QPushButton::clicked,
            this,
            &ImageRotationGUI::onRotateLeftButtonClicked);
    connect(ui->rotateRightButton,
            &QPushButton::clicked,
            this,
            &ImageRotationGUI::onRotateRightButtonClicked);
    connect(ui->hFlipButton,
            &QPushButton::clicked,
            this,
            &ImageRotationGUI::onFlipHorizontalButtonClicked);
    connect(ui->vFlipButton,
            &QPushButton::clicked,
            this,
            &ImageRotationGUI::onFlipVerticalButtonClicked);
}

ImageRotationGUI::~ImageRotationGUI()
{
    delete ui;
}

void ImageRotationGUI::onLoadImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    bool result = imageRotation->load(path);
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()), true);

    // TODO: load()の結果に応じて何かメッセージを出す
}

void ImageRotationGUI::onSaveImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    if (imageRotation->overwriteSave(path)) {
        QMessageBox::information(this, tr("Save Successful"), tr("Image was saved successfully."));
    } else {
        QMessageBox::critical(this, tr("Save Failed"), tr("Failed to save the image."));
    }
}

void ImageRotationGUI::onResetButtonClicked()
{
    imageRotation->reset();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}

void ImageRotationGUI::onRotateRightButtonClicked()
{
    if (imageRotation->original().isNull())
        return;

    imageRotation->rotateDegrees(-90);
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}

void ImageRotationGUI::onRotateLeftButtonClicked()
{
    if (imageRotation->original().isNull())
        return;

    imageRotation->rotateDegrees(90);
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}

void ImageRotationGUI::onFlipHorizontalButtonClicked()
{
    if (imageRotation->original().isNull())
        return;

    imageRotation->flipHorizontal();
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}

void ImageRotationGUI::onFlipVerticalButtonClicked()
{
    if (imageRotation->original().isNull())
        return;

    imageRotation->flipVertical();
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}
