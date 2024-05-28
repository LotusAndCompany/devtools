#include "home.h"
#include "ui_home.h"

home::home(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::home)
{
    ui->setupUi(this);

    ui->templateText->setHidden(true);
    ui->templateTitle->setHidden(true);
    // ui->pushButton->setHidden(true);
    // ui->pushButton->raise();
    ui->titleList->raise();
    ui->titleList->setHidden(true);
    ui->closeTitleListButton->setHidden(true);
    ui->label->setHidden(true);
    ui->label_2->setHidden(true);
    ui->label_3->setHidden(true);
    ui->label_4->setHidden(true);

}

home::~home()
{
    delete ui;
}

void home::on_addButton_clicked()
{
    ui->templateText->setHidden(false);
    ui->templateTitle->setHidden(false);
    // ui->pushButton->setHidden(false);
}


void home::on_showTitleListButton_clicked()
{
    ui->titleList->setHidden(false);
    ui->closeTitleListButton->setHidden(false);
    ui->label->setHidden(false);
    ui->label_2->setHidden(false);
    ui->label_3->setHidden(false);
    ui->label_4->setHidden(false);
}


void home::on_closeTitleListButton_clicked()
{
    ui->titleList->setHidden(true);
    ui->closeTitleListButton->setHidden(true);
    ui->label->setHidden(true);
    ui->label_2->setHidden(true);
    ui->label_3->setHidden(true);
    ui->label_4->setHidden(true);
}

