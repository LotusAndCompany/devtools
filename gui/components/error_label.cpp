#include "gui/components/error_label.h"

StatusLabel::StatusLabel(QWidget *parent) : QLabel(parent), m_level(Level::Error)
{
    setVisible(false);
    setStyleSheet(kErrorStyle);
}

void StatusLabel::setLevel(Level level)
{
    m_level = level;
    setStyleSheet(styleForLevel(level));
}

void StatusLabel::showMessage(Level level, const QString &message)
{
    setLevel(level);
    setText(message);
    setVisible(true);
}

void StatusLabel::clear()
{
    setVisible(false);
    QLabel::clear();
}

const char *StatusLabel::styleForLevel(Level level)
{
    switch (level) {
    case Level::Error:
        return kErrorStyle;
    case Level::Warning:
        return kWarningStyle;
    case Level::Success:
        return kSuccessStyle;
    }
    return kErrorStyle;
}
