#include "sidemenu.h"
#include "ui_sidemenu.h"

Sidemenu::Sidemenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Sidemenu)
{
    ui->setupUi(this);
}

Sidemenu::~Sidemenu()
{
    delete ui;
}
