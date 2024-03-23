#include "contents_area.h"
#include "ui_contents_area.h"

ContentsArea::ContentsArea(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ContentsArea)
{
    ui->setupUi(this);
}

ContentsArea::~ContentsArea()
{
    delete ui;
}
