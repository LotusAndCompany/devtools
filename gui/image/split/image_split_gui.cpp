#include "image_split_gui.h"
#include "ui_image_split_gui.h"

#include "core/image/split/image_split.h"

ImageSplitGUI::ImageSplitGUI(ImageSplitInterface *imageSplit, QWidget *parent)
    : QWidget(parent)
    , imageSplit(imageSplit)
    , ui(new Ui::ImageSplitGUI)
{
    ui->setupUi(this);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageSplit->parent() == nullptr)
        imageSplit->setParent(this);
}

ImageSplitGUI::~ImageSplitGUI()
{
    delete ui;
}
