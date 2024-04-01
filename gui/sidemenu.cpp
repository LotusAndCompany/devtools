#include "sidemenu.h"
#include "ui_sidemenu.h"

#include <QButtonGroup>
#include "core/enum_cast.h"
#include "core/exception/invalid_argument_exception.h"
#include "core/exception/under_development_exception.h"
#include "sidemenu_item.h"

const QString Sidemenu::invalidSidemenuIDReason = QString("Sidemenu::ID must be in range (%1, %2)")
                                                      .arg(Sidemenu::ID_UNDEFINED)
                                                      .arg(Sidemenu::ID_MAX);
const QMap<Sidemenu::ID, QString> Sidemenu::iconNames = {
    {ID::UNDEFINED, "question_mark"}, // NOTE: unused
    {ID::HOME, "home"},               // NOTE: unused, Home icon is set in sidemenu.ui
    // NOTE: sample items
    {ID::SAMPLE_0, "counter_0"},
    {ID::SAMPLE_1, "counter_1"},
    {ID::SAMPLE_2, "counter_2"},
    {ID::SAMPLE_3, "counter_3"},
    {ID::MAX, "question_mark"}, // NOTE: unused
};

Sidemenu::Sidemenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Sidemenu)
    , buttonGroup(new QButtonGroup(this))
{
    ui->setupUi(this);

    buttonGroup->setExclusive(true);

    connect(buttonGroup, &QButtonGroup::idToggled, this, &Sidemenu::onButtonToggled);

    buttonGroup->addButton(ui->homeButton, static_cast<int>(ID::HOME));

    // WIP: 適当なボタンを追加する
    registerItem(ID::SAMPLE_0);
    registerItem(ID::SAMPLE_1);
    registerItem(ID::SAMPLE_2);
    registerItem(ID::SAMPLE_3);

    ui->scrollAreaLayout->addStretch();
}

Sidemenu::~Sidemenu()
{
    delete ui;
}

void Sidemenu::validateID(Sidemenu::ID id)
{
    const int intID = static_cast<int>(id);

    if (intID <= ID_UNDEFINED || ID_MAX <= intID)
        throw InvalidArgumentException(intID, invalidSidemenuIDReason);
}

const QIcon Sidemenu::icon(Sidemenu::ID id)
{
    validateID(id);

    if (iconNames.contains(id))
        return QIcon::fromTheme(iconNames[id]);
    else
        throw UnderDevelopmentException();
}

void Sidemenu::registerItem(ID id)
{
    SidemenuItem *const item = new SidemenuItem(id, this);
    buttonGroup->addButton(item, static_cast<int>(id));
    ui->scrollAreaLayout->addWidget(item);
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

void Sidemenu::onButtonToggled(int intID, bool checked)
{
    // NOTE: signal/slotでは例外を投げるべきではない
    try {
        const ID id = enum_cast<ID>(intID);

        validateID(id);

        if (checked)
            emit itemSelected(id);
    } catch (InvalidArgumentException<int> &e) {
        qWarning() << e.message;
    }
}
