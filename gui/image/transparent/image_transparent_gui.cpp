#include "image_transparent_gui.h"
#include "ui_image_transparent_gui.h"

ImageTransparentGUI::ImageTransparentGUI(ImageTransparentInterface *imageTransparentQWidget,
                                         QWidget *parent)
    : GuiTool(parent)
    , ui(new Ui::ImageTransparentGUI)
    , imageTransparent(imageTransparent)
{
    ui->setupUi(this);
}

ImageTransparentGUI::~ImageTransparentGUI()
{
    delete ui;
}
