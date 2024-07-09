#include "ItemWidget.h"
#include "ui_ItemWidget.h"

ItemWidget::ItemWidget(const QString &title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemWidget),
    title(title)
{
    ui->setupUi(this);
    ui->listTitleLabel->setText(title);
}

ItemWidget::~ItemWidget()
{
    delete ui;
}

QString ItemWidget::getTitle() const
{
    return title;
}

// void ItemWidget::on_copyButton_clicked()
// {
//     emit copyButtonClicked(title);
// }

void ItemWidget::on_listCopyButton_clicked()
{
    emit listCopyButtonClicked(title);
}

