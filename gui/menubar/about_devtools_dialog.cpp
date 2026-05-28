#include "about_devtools_dialog.h"

// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "app_info.autogen.cpp"

#include <QDialogButtonBox>
#include <QFile>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QLabel *createCaptionLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
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
    resize(640, 480);

    auto *rootLayout = new QVBoxLayout(this);

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

    auto *titleLayout = new QHBoxLayout;
    titleLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
    titleLayout->addSpacerItem(
        new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    auto *appLogo = new QLabel(tab);
    appLogo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    appLogo->setMinimumSize(64, 64);
    appLogo->setMaximumSize(64, 64);
    appLogo->setPixmap(QPixmap(QStringLiteral(":/logo/dev-tools_logo.png")));
    appLogo->setScaledContents(true);
    titleLayout->addWidget(appLogo);

    auto *appName = new QLabel(QStringLiteral("DevTools"), tab);
    QFont appNameFont = appName->font();
    appNameFont.setPointSize(48);
    appNameFont.setBold(true);
    appName->setFont(appNameFont);
    appName->setTextFormat(Qt::AutoText);
    titleLayout->addWidget(appName);

    titleLayout->addSpacerItem(
        new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    layout->addLayout(titleLayout);

    auto *gridLayout = new QGridLayout;
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 2);

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
    copyButtonLayout->setContentsMargins(copyButtonLayout->contentsMargins().left(),
                                         copyButtonLayout->contentsMargins().top(),
                                         copyButtonLayout->contentsMargins().right(), 0);
    copyButtonLayout->addSpacerItem(
        new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    auto *copyButton = new QPushButton(tr("Copy"), tab);
    copyButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    copyButton->setLayoutDirection(Qt::LeftToRight);
    copyButtonLayout->addWidget(copyButton);
    layout->addLayout(copyButtonLayout);

    layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    connect(copyButton, &QPushButton::clicked, this, &AboutDevToolsDialog::onCopyButtonClicked);
    return tab;
}

QWidget *AboutDevToolsDialog::createLicenseTab(QWidget *parent)
{
    auto *tab = new QWidget(parent);
    auto *layout = new QVBoxLayout(tab);
    auto *licenseText = new QPlainTextEdit(tab);
    licenseText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
