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

void ContentsArea::changeEvent(QEvent *event)
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
