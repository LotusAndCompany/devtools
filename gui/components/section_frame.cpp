#include "gui/components/section_frame.h"

#include <QCheckBox>
#include <QVBoxLayout>

SectionFrame::SectionFrame(const QString &title, QWidget *parent)
    : QFrame(parent)
    , m_toggle(new QCheckBox(title, this))
    , m_body(nullptr)
    , m_layout(new QVBoxLayout(this))
    , m_title(title)
{
    setFrameShape(QFrame::StyledPanel);
    m_layout->addWidget(m_toggle);

    m_toggle->setChecked(true);

    connect(m_toggle, &QCheckBox::checkStateChanged, this, [this]() {
        const bool expanded = m_toggle->isChecked();
        updateToggleText(expanded);
        if (m_body) {
            m_body->setVisible(expanded);
        }
        emit expandedChanged(expanded);
    });

    updateToggleText(true);
}

void SectionFrame::setBody(QWidget *body)
{
    if (m_body) {
        m_layout->removeWidget(m_body);
        m_body->deleteLater();
    }
    m_body = body;
    if (m_body) {
        m_body->setParent(this);
        m_layout->addWidget(m_body);
        m_body->setVisible(m_toggle->isChecked());
    }
}

QWidget *SectionFrame::body() const
{
    return m_body;
}

bool SectionFrame::isExpanded() const
{
    return m_toggle->isChecked();
}

void SectionFrame::setExpanded(bool expanded)
{
    m_toggle->setChecked(expanded);
}

void SectionFrame::updateToggleText(bool expanded)
{
    m_toggle->setText((expanded ? QStringLiteral("\u25BC ") : QStringLiteral("\u25B6 ")) + m_title);
}
