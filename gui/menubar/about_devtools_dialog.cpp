#include "about_devtools_dialog.h"
#include "app_info.autogen.cpp"
#include "ui_about_devtools_dialog.h"

#include <QPushButton>

AboutDevToolsDialog::AboutDevToolsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDevToolsDialog)
{
    ui->setupUi(this);

    ui->appVersion->setText(DevTools::APP_VERSION);
    ui->revision->setText(DevTools::GIT_REVISION);
    ui->buildEnv->setText(QString(DevTools::OS_NAME) + " " + QString(DevTools::OS_VERSION) + ", "
                          + QString(DevTools::COMPILER_NAME) + " "
                          + QString(DevTools::COMPILER_VERSION));
    ui->qtVersion->setText(qVersion());

    ui->qtInfo->setText(QString("Qt %1: LGPL v3").arg(qVersion()));

    connect(ui->copyButton, &QPushButton::clicked, this, &AboutDevToolsDialog::onCopyButtonClicked);
}

AboutDevToolsDialog::~AboutDevToolsDialog()
{
    delete ui;
}

void AboutDevToolsDialog::onCopyButtonClicked()
{
    // TODO: copy to clip board
    qDebug() << "Copy";
}
