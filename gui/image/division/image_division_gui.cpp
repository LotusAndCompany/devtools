#include "image_division_gui.h"
#include "ui_image_division_gui.h"

#include "core/image/division/image_division.h"

ImageDivisionGUI::ImageDivisionGUI(ImageDivisionInterface *imageDivision, QWidget *parent)
    : QWidget(parent)
    , imageDivision(imageDivision)
    , ui(new Ui::ImageDivisionGUI)
{
    ui->setupUi(this);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageDivision->parent() == nullptr)
        imageDivision->setParent(this);
}

ImageDivisionGUI::~ImageDivisionGUI()
{
    delete ui;
}
