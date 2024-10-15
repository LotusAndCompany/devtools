#include "sidemenu.h"
#include "ui_sidemenu.h"

#include <QButtonGroup>
#include "core/enum_cast.h"
#include "core/exception/invalid_argument_exception.h"
#include "core/exception/under_development_exception.h"
#include "sidemenu_item.h"

const QString Sidemenu::invalidSidemenuIDReason = QString("Sidemenu::ID must be in range (%1, %2)")
                                                      .arg(Sidemenu::ID_MIN)
                                                      .arg(Sidemenu::ID_MAX);

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
    registerItem(ID::IMAGE_RESIZE);
    registerItem(ID::IMAGE_ROTATION);
    registerItem(ID::IMAGE_DIVISION);

    ui->scrollAreaLayout->addStretch();
}

Sidemenu::~Sidemenu()
{
    delete ui;
}

void Sidemenu::validateID(Sidemenu::ID id)
{
    const int intID = static_cast<int>(id);

    if (intID <= ID_MIN || ID_MAX <= intID)
        throw InvalidArgumentException(intID, invalidSidemenuIDReason);
}

const QIcon Sidemenu::icon(Sidemenu::ID id)
{
    validateID(id);

    QString iconName;
    switch (id) {
    case ID::HOME:
        // NOTE: unused, Home icon is set in sidemenu.ui
        qWarning() << "Home icon is set in sidemenu.ui";
        iconName = "home";
        break;

        // NOTE: sample items
    case ID::SAMPLE_0:
        iconName = "counter_0";
        break;
    case ID::SAMPLE_1:
        iconName = "counter_1";
        break;
    case ID::SAMPLE_2:
        iconName = "counter_2";
        break;
    case ID::SAMPLE_3:
        iconName = "counter_3";
        break;
    case ID::IMAGE_RESIZE:
    case ID::IMAGE_ROTATION:
    case ID::IMAGE_DIVISION:
        iconName = "division";
        break;

    default:
        throw UnderDevelopmentException();
    }

    return QIcon::fromTheme(iconName);
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
