#include "sidemenu_item.h"

#include "features/framework/core/enum_cast.h"
#include "features/framework/core/exception/invalid_argument_exception.h"
#include "features/framework/core/tool/tool.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionButton>

const QString SidemenuItem::notConfigurableReason = "Sidemenu::ID::HOME is not confugurable";

SidemenuItem::SidemenuItem(Sidemenu::ID id, QWidget *parent) : QPushButton(parent), id(id)
{
    setFlat(true);
    setCheckable(true);
    setStyleSheet("text-align:left;");
    setIconSize(QSize(20, 20));
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

    const QRect contentsRect =
        style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
    const int iconWidth = option.icon.isNull() ? 0 : option.iconSize.width();
    constexpr int ICON_TEXT_SPACING = 6;
    const int availableTextWidth = contentsRect.width() - iconWidth - ICON_TEXT_SPACING;
    option.text = fontMetrics().elidedText(option.text, Qt::ElideRight, availableTextWidth,
                                           Qt::TextSingleLine);

    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
}
