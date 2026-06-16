#include "gui/components/action_button.h"

ActionButton::ActionButton(const QIcon &icon, const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    if (!icon.isNull()) {
        setIcon(icon);
    }
    setStyleSheet(kPaddingStyle);
}

void ActionButton::setIcon(const QIcon &icon)
{
    QPushButton::setIcon(icon);
}

void ActionButton::setText(const QString &text)
{
    QPushButton::setText(text);
}
