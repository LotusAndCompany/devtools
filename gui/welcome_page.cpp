#include "welcome_page.h"

#include "ui_welcome_page.h"

WelcomePage::WelcomePage(QWidget *parent) : QWidget(parent), ui(new Ui::WelcomePage)
{
    ui->setupUi(this);
}

WelcomePage::~WelcomePage()
{
    delete ui;
}

void WelcomePage::changeEvent(QEvent *event)
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
