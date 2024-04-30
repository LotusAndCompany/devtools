#include "image_rotation_gui.h"
#include "ui_image_rotation_gui.h"

#include "core/image/rotation/image_rotation.h"

ImageRotationGUI::ImageRotationGUI(ImageRotationInterface *imageRotation, QWidget *parent)
    : GuiTool(parent)
    , imageRotation(imageRotation)
    , ui(new Ui::ImageRotationGUI)
{
    ui->setupUi(this);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageRotation->parent() == nullptr)
        imageRotation->setParent(this);
}

ImageRotationGUI::~ImageRotationGUI()
{
    delete ui;
}
