#include "sidemenu_item.h"

#include "features/framework/core/enum_cast.h"
#include "features/framework/core/exception/invalid_argument_exception.h"
#include "features/framework/core/tool/tool.h"
#include "features/framework/gui/design_system.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionButton>

const QString SidemenuItem::notConfigurableReason = "Sidemenu::ID::HOME is not confugurable";

SidemenuItem::SidemenuItem(Sidemenu::ID id, QWidget *parent) : QPushButton(parent), id(id)
{
    DevTools::Ui::configureSidebarItem(this);
    setCheckable(true);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);

    Sidemenu::validateID(id);

    if (id == Sidemenu::ID::WELCOME) {
        throw InvalidArgumentException(static_cast<int>(id), notConfigurableReason);
    }

    setIcon(Sidemenu::icon(id));
    // NOTE: Tool::IDとSidemenu::IDはHOME, MAX以外は一致する想定
    setText(Tool::translatable(enum_cast<Tool::ID>(static_cast<int>(id))).name);
}

void SidemenuItem::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        // アイコンは変更しない想定

        // NOTE: Tool::IDとSidemenu::IDはHOME, MAX以外は一致する想定
        setText(Tool::translatable(enum_cast<Tool::ID>(static_cast<int>(id))).name);
        event->accept();
        break;
    default:
        QPushButton::changeEvent(event);
        break;
    }
}

void SidemenuItem::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    QStyleOptionButton option;
    initStyleOption(&option);

    const auto textRole =
        (option.state & QStyle::State_On) != 0 ? QPalette::HighlightedText : QPalette::Text;
    const auto colorGroup =
        (option.state & QStyle::State_Enabled) != 0 ? QPalette::Normal : QPalette::Disabled;
    option.palette.setColor(QPalette::All, QPalette::ButtonText,
                            style()->standardPalette().color(colorGroup, textRole));

    constexpr int ICON_TEXT_SPACING = 4;
    const int iconWidth = option.icon.isNull() ? 0 : option.iconSize.width();
    const int availableTextWidth =
        style()->subElementRect(QStyle::SE_PushButtonContents, &option, this).width() - iconWidth -
        (iconWidth > 0 && !option.text.isEmpty() ? ICON_TEXT_SPACING : 0);
    option.text = availableTextWidth > 0
                      ? fontMetrics().elidedText(option.text, Qt::ElideRight, availableTextWidth,
                                                 Qt::TextSingleLine)
                      : QString();

    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
}
