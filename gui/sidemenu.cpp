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
    : QWidget(parent), ui(new Ui::Sidemenu), buttonGroup(new QButtonGroup(this))
{
    ui->setupUi(this);

    buttonGroup->setExclusive(true);

    connect(buttonGroup, &QButtonGroup::idToggled, this, &Sidemenu::onButtonToggled);
    connect(ui->searchBoxEdit, &QLineEdit::textChanged, this, &Sidemenu::onSearchTextChanged);

    // WIP: 適当なボタンを追加する
    registerItem(ID::IMAGE_RESIZE);
    registerItem(ID::IMAGE_ROTATION);
    registerItem(ID::IMAGE_DIVISION);
    registerItem(ID::IMAGE_TRANSPARENT);
    registerItem(ID::PHRASE_GENERATION);
    registerItem(ID::COMMAND_GENERATION);
    registerItem(ID::HTTP_REQUEST);
    registerItem(ID::DATA_CONVERSION);

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
    switch (id)
    {
    case ID::HTTP_REQUEST:
        iconName = "network";
        break;
    case ID::IMAGE_RESIZE:
        iconName = "resize";
        break;
    case ID::IMAGE_ROTATION:
        iconName = "clockwise";
        break;
    case ID::IMAGE_DIVISION:
        iconName = "division";
        break;
    case ID::IMAGE_TRANSPARENT:
        iconName = "transparent";
        break;
    case ID::PHRASE_GENERATION:
        iconName = "library_books";
        break;
    case ID::COMMAND_GENERATION:
        iconName = "terminal";
        break;
    case ID::DATA_CONVERSION:
        iconName = "question_mark";
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
    allItems.append(item);
}

void Sidemenu::changeEvent(QEvent *event)
{
    switch (event->type())
    {
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
    try
    {
        const ID id = enum_cast<ID>(intID);

        validateID(id);

        if (checked)
            emit itemSelected(id);
    }
    catch (InvalidArgumentException<int> &e)
    {
        qWarning() << e.message;
    }
}

void Sidemenu::filterItems(const QString &searchText)
{
    const QString lowerSearchText = searchText.toLower();
    
    for (SidemenuItem *item : allItems) {
        const QString itemText = item->text().toLower();
        const bool shouldShow = searchText.isEmpty() || itemText.contains(lowerSearchText);
        item->setVisible(shouldShow);
    }
}

void Sidemenu::onSearchTextChanged(const QString &text)
{
    filterItems(text);
}
