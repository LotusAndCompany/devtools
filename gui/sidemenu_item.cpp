#include "sidemenu_item.h"

#include <QMouseEvent>
#include "core/enum_cast.h"
#include "core/exception/invalid_argument_exception.h"
#include "core/tool/tool.h"

const QString SidemenuItem::notConfigurableReason = "Sidemenu::ID::HOME is not confugurable";

SidemenuItem::SidemenuItem(Sidemenu::ID id, QWidget *parent)
    : QPushButton(parent)
    , id(id)
{
    setFlat(true);
    setCheckable(true);
    setStyleSheet("text-align: left;");
    setIconSize(QSize(20, 20));
    setFocusPolicy(Qt::FocusPolicy::NoFocus);

    Sidemenu::validateID(id);

    if (id == Sidemenu::ID::HOME)
        throw InvalidArgumentException(static_cast<int>(id), notConfigurableReason);

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
