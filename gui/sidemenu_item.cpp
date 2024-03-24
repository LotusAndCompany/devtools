#include "sidemenu_item.h"

#include <QMouseEvent>
#include "gui/gui_application.h"

SidemenuItem::SidemenuItem(QWidget *parent)
    : QPushButton(parent)
{
    setFlat(true);
    setCheckable(true);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

SidemenuItem::SidemenuItem(const QIcon &icon, const QString &text, QWidget *parent)
    : SidemenuItem(parent)
{
    setIcon(icon);
    setText(text);
}

void SidemenuItem::mouseReleaseEvent(QMouseEvent *e)
{
    // ユーザー操作で選択解除させない
    if (e->button() == Qt::LeftButton) {
        setChecked(true);
        return;
    }
    QPushButton::mouseReleaseEvent(e);
}

// TODO: 翻訳を実装する
void SidemenuItem::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        //ui->retranslateUi(this);
        //setText(GuiApplication::tran);
        event->accept();
        break;
    default:
        QPushButton::changeEvent(event);
        break;
    }
}
