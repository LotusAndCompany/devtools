#include "gui/components/labeled_spinbox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

LabeledSpinbox::LabeledSpinbox(const QString &label, QWidget *parent)
    : QWidget(parent), m_label(new QLabel(label, this)), m_spinBox(new QSpinBox(this))
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);
    layout->addWidget(m_spinBox);
    layout->addStretch();

    m_label->setBuddy(m_spinBox);

    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &LabeledSpinbox::valueChanged);
}

void LabeledSpinbox::setRange(int min, int max)
{
    m_spinBox->setRange(min, max);
}

int LabeledSpinbox::value() const
{
    return m_spinBox->value();
}

void LabeledSpinbox::setValue(int value)
{
    m_spinBox->setValue(value);
}

QSpinBox *LabeledSpinbox::spinBox() const
{
    return m_spinBox;
}
