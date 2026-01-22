#include "image_main.h"
#include "ui_image_main.h"

ImageMain::ImageMain(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ImageMain)
{
    ui->setupUi(this);
}

ImageMain::~ImageMain()
{
    delete ui;
}
