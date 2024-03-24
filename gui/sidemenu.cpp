#include "sidemenu.h"
#include "ui_sidemenu.h"

#include <QButtonGroup>

Sidemenu::Sidemenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Sidemenu)
    , buttonGroup(new QButtonGroup(this))
{
    ui->setupUi(this);

    buttonGroup->addButton(ui->homeButton, static_cast<int>(ItemID::HOME));
}

Sidemenu::~Sidemenu()
{
    delete ui;
}

void Sidemenu::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}
