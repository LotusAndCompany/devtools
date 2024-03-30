#include "sidemenu.h"
#include "ui_sidemenu.h"

#include <QButtonGroup>

Sidemenu::Sidemenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Sidemenu)
    , buttonGroup(new QButtonGroup(this))
{
    ui->setupUi(this);

    buttonGroup->setExclusive(true);

    connect(buttonGroup, &QButtonGroup::idToggled, this, &Sidemenu::onButtonToggled);

    //SidemenuItem::UiData::initializeIcon();
    //SidemenuItem::UiData::initializeText();

    ui->homeButton->setChecked(true);
    ui->homeButton->configure(ID::HOME);
    buttonGroup->addButton(ui->homeButton, static_cast<int>(ID::HOME));

    // WIP: 適当なボタンを追加する
    /*
    registerItem(ItemID::SAMPLE_0);
    registerItem(ItemID::SAMPLE_1);
    registerItem(ItemID::SAMPLE_2);
    registerItem(ItemID::SAMPLE_3);
    */

    ui->scrollAreaLayout->addStretch();
}

Sidemenu::~Sidemenu()
{
    delete ui;
}

void Sidemenu::registerItem(ID id)
{
    /*
    const auto &uiData = SidemenuItem::UiData::list[id];
    SidemenuItem *const item = new SidemenuItem(uiData.icon, uiData.text, this);
    item->configure(id);
    buttonGroup->addButton(item, static_cast<int>(id));
    ui->scrollAreaLayout->addWidget(item);
    */
}

void Sidemenu::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void Sidemenu::onButtonToggled(int id, bool checked)
{
    if (id == ID_UNDEFINED) {
        qWarning() << "This item is not configured";
        return;
    }
    if (id < ID_UNDEFINED || ID_MAX <= id) {
        qWarning() << "Invalid button id:" << id;
        return;
    }
    if (checked)
        emit itemSelected(static_cast<ID>(id));
}
