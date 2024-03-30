#include "sidemenu_item.h"

#include <QMouseEvent>
#include "gui_application.h"

//QString SidemenuItem::UiData::currentLanguage = "";
//QMap<Sidemenu::ID, SidemenuItem::UiData> SidemenuItem::UiData::list;

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
    setStyleSheet("text-align:left;");
}

/*
void SidemenuItem::UiData::initializeIcon()
{
    list[Sidemenu::ID::UNDEFINED].icon = QIcon::fromTheme("question_mark");
    list[Sidemenu::ID::HOME].icon = QIcon::fromTheme("home");
    list[Sidemenu::ItemID::SAMPLE_0].icon = QIcon::fromTheme("counter_0");
    list[Sidemenu::ItemID::SAMPLE_1].icon = QIcon::fromTheme("counter_1");
    list[Sidemenu::ItemID::SAMPLE_2].icon = QIcon::fromTheme("counter_2");
    list[Sidemenu::ItemID::SAMPLE_3].icon = QIcon::fromTheme("counter_3");
}

void SidemenuItem::UiData::initializeText()
{
    list[Sidemenu::ID::UNDEFINED].text = "<Undefined>";
    list[Sidemenu::ID::HOME].text = tr("Home");
    list[Sidemenu::ItemID::SAMPLE_0].text = tr("Sample 0");
    list[Sidemenu::ItemID::SAMPLE_1].text = tr("Sample 1");
    list[Sidemenu::ItemID::SAMPLE_2].text = tr("Sample 2");
    list[Sidemenu::ItemID::SAMPLE_3].text = tr("Sample 3");

    // WARNING: 型チェックをしないのでGuiApplicationの時以外は呼んではいけない
    currentLanguage = static_cast<GuiApplication *>(qApp)->language();
}

bool SidemenuItem::UiData::isListValid()
{
    // WARNING: 型チェックをしないのでGuiApplicationの時以外は呼んではいけない
    return !list.empty() && static_cast<GuiApplication *>(qApp)->language() == currentLanguage;
}
*/

void SidemenuItem::configure(Sidemenu::ID id)
{
    if (isConfigured())
        qWarning() << "This item is already configured. current id:" << static_cast<int>(this->id);

    this->id = id;

    //setIcon(UiData::list[id].icon);
    setIconSize(QSize(20, 20));
    //setText(UiData::list[id].text);
}

void SidemenuItem::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        if (isConfigured()) {
            /*
            if (!UiData::isListValid())
                UiData::initializeText();

            // アイコンは変更しない想定
            setText(UiData::list[id].text);
            */
        } else {
            qWarning() << "SidemenuItem is not configured";
        }
        event->accept();
        break;
    default:
        QPushButton::changeEvent(event);
        break;
    }
}
