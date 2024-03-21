#include "about_devtools_dialog.h"
#include "ui_about_devtools_dialog.h"

AboutDevToolsDialog::AboutDevToolsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDevToolsDialog)
{
    ui->setupUi(this);
}

AboutDevToolsDialog::~AboutDevToolsDialog()
{
    delete ui;
}
