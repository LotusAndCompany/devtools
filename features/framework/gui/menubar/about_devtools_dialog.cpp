#include "about_devtools_dialog.h"

#include "../design_system.h"

// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "app_info.autogen.cpp"

#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QLabel *createCaptionLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    DevTools::Ui::configureCaptionLabel(label);
    label->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByKeyboard |
                                   Qt::TextSelectableByMouse);
    return label;
}

QLabel *createValueLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByKeyboard |
                                   Qt::TextSelectableByMouse);
    return label;
}

} // namespace

AboutDevToolsDialog::AboutDevToolsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("About DevTools"));
    DevTools::Ui::configureDialog(this);

    auto *rootLayout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(rootLayout);

    auto *tabWidget = new QTabWidget(this);
    tabWidget->addTab(createAboutTab(tabWidget), tr("About"));
    tabWidget->addTab(createLicenseTab(tabWidget), tr("License"));
    tabWidget->setCurrentIndex(0);

    rootLayout->addWidget(tabWidget);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    buttonBox->setCenterButtons(true);
    rootLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QWidget *AboutDevToolsDialog::createAboutTab(QWidget *parent)
{
    auto *tab = new QWidget(parent);
    auto *layout = new QVBoxLayout(tab);
    DevTools::Ui::applyPanelLayout(layout);

    auto *titleLayout = new QHBoxLayout;
    titleLayout->addStretch();

    auto *appLogo = new QLabel(tab);
    DevTools::Ui::configureLogoLabel(appLogo);
    appLogo->setPixmap(QPixmap(QStringLiteral(":/logo/dev-tools_logo.png")));
    titleLayout->addWidget(appLogo);

    auto *appName = new QLabel(QStringLiteral("DevTools"), tab);
    DevTools::Ui::configureDisplayTitle(appName);
    appName->setTextFormat(Qt::AutoText);
    titleLayout->addWidget(appName);

    titleLayout->addStretch();

    layout->addLayout(titleLayout);

    auto *gridLayout = new QGridLayout;
    DevTools::Ui::applyInlineLayout(gridLayout);
    DevTools::Ui::configureCaptionValueGrid(gridLayout);

    const QString buildEnvText = QString::fromUtf8(DevTools::OS_NAME) + QStringLiteral(" ") +
                                 QString::fromUtf8(DevTools::OS_VERSION) + QStringLiteral(", ") +
                                 QString::fromUtf8(DevTools::COMPILER_NAME) + QStringLiteral(" ") +
                                 QString::fromUtf8(DevTools::COMPILER_VERSION);

    gridLayout->addWidget(createCaptionLabel(tr("Application Version"), tab), 0, 0);
    gridLayout->addWidget(createValueLabel(QString::fromUtf8(DevTools::APP_VERSION), tab), 0, 1);
    gridLayout->addWidget(createCaptionLabel(tr("Revision"), tab), 1, 0);
    gridLayout->addWidget(createValueLabel(QString::fromUtf8(DevTools::GIT_REVISION), tab), 1, 1);
    gridLayout->addWidget(createCaptionLabel(tr("Build Environment"), tab), 2, 0);
    gridLayout->addWidget(createValueLabel(buildEnvText, tab), 2, 1);
    gridLayout->addWidget(createCaptionLabel(tr("Qt Version"), tab), 3, 0);
    gridLayout->addWidget(createValueLabel(QString::fromUtf8(qVersion()), tab), 3, 1);
    gridLayout->addWidget(createCaptionLabel(QStringLiteral("Copyright"), tab), 4, 0);
    gridLayout->addWidget(
        createValueLabel(QStringLiteral("2025 © Lotus&Company Inc. All rights reserved."), tab), 4,
        1);

    layout->addLayout(gridLayout);

    auto *copyButtonLayout = new QHBoxLayout;
    auto *copyButton = new QPushButton(tr("Copy"), tab);
    DevTools::Ui::configureCompactButton(copyButton);
    copyButtonLayout->addWidget(copyButton);
    DevTools::Ui::configureActionBar(copyButtonLayout, DevTools::Ui::ActionBarAlignment::Trailing);
    layout->addLayout(copyButtonLayout);

    layout->addStretch();

    connect(copyButton, &QPushButton::clicked, this, &AboutDevToolsDialog::onCopyButtonClicked);
    return tab;
}

QWidget *AboutDevToolsDialog::createLicenseTab(QWidget *parent)
{
    auto *tab = new QWidget(parent);
    auto *layout = new QVBoxLayout(tab);
    DevTools::Ui::applyPanelLayout(layout);
    auto *licenseText = DevTools::Ui::createPlainTextEdit(tab);
    DevTools::Ui::configureTextControl(licenseText);
    licenseText->setReadOnly(true);

    QFile licenseFile(QStringLiteral(":/docs/LICENSE"));
    if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        licenseText->setPlainText(QString::fromUtf8(licenseFile.readAll()));
    } else {
        licenseText->setPlainText(tr("Failed to load license information."));
    }
    layout->addWidget(licenseText);
    return tab;
}

void AboutDevToolsDialog::onCopyButtonClicked()
{
    // TODO: copy to clip board
    qDebug() << "Copy";
}
