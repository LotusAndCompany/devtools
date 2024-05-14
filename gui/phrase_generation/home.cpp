#include "home.h"
#include "ui_home.h"

home::home(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::home)
{
    ui->setupUi(this);
    // ui->templateText->hide();
    // ui->templateTitle->hide();
    ui->templateText->setHidden(true);
    ui->templateTitle->setHidden(true);
    ui->collectionListArea->raise();
    ui->collectionListArea->setHidden(true);
    ui->closeCollectionListButton->setHidden(true);
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
    // ui->templateText->show();
    // ui->templateTitle->show();
    ui->templateText->setHidden(false);
    ui->templateTitle->setHidden(false);
}


void home::on_showCollectionListButton_clicked()
{
    ui->collectionListArea->setHidden(false);
    ui->closeCollectionListButton->setHidden(false);
    ui->label->setHidden(false);
    ui->label_2->setHidden(false);
    ui->label_3->setHidden(false);
    ui->label_4->setHidden(false);
}


void home::on_closeCollectionListButton_clicked()
{
    ui->collectionListArea->setHidden(true);
    ui->closeCollectionListButton->setHidden(true);
    ui->label->setHidden(true);
    ui->label_2->setHidden(true);
    ui->label_3->setHidden(true);
    ui->label_4->setHidden(true);
}

