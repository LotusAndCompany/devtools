#include "about_devtools_dialog.h"

// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "app_info.autogen.cpp"
#include "ui_about_devtools_dialog.h"

#include <QFile>
#include <QPushButton>

AboutDevToolsDialog::AboutDevToolsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDevToolsDialog)
{
    ui->setupUi(this);

    ui->appVersion->setText(DevTools::APP_VERSION);
    ui->revision->setText(DevTools::GIT_REVISION);
    ui->buildEnv->setText(QString(DevTools::OS_NAME) + " " + QString(DevTools::OS_VERSION) + " (" +
                          QString(DevTools::TARGET_ARCH) + "), " +
                          QString(DevTools::COMPILER_NAME) + " " +
                          QString(DevTools::COMPILER_VERSION));
    ui->qtVersion->setText(qVersion());

    QFile licenseFile(QStringLiteral(":/docs/LICENSE"));
    if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString licenseText = QString::fromUtf8(licenseFile.readAll());
        ui->licenseText->setPlainText(licenseText);
    } else {
        ui->licenseText->setPlainText(tr("Failed to load license information."));
    }

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
