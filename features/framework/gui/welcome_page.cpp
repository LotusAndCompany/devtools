#include "welcome_page.h"

#include "design_system.h"

#include <QEvent>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

WelcomePage::WelcomePage(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("DevTools - Welcome"));

    auto *const layout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(layout);
    layout->addStretch();

    auto *const logoLabel = new QLabel(this);
    logoLabel->setPixmap(QPixmap(":/logo/dev-tools_logo.png"));
    DevTools::Ui::configureCenteredLabel(logoLabel);
    layout->addWidget(logoLabel);

    m_messageLabel = new QLabel(this);
    DevTools::Ui::configureHeroLabel(m_messageLabel);
    layout->addWidget(m_messageLabel);

    layout->addStretch();

    retranslateUi();
}

void WelcomePage::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void WelcomePage::retranslateUi()
{
    setWindowTitle(tr("DevTools - Welcome"));
    m_messageLabel->setText(tr("Welcome to DevTools!\nChoose a tool from the side panel"));
}
