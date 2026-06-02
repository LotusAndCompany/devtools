#include "sidemenu.h"

#include "core/enum_cast.h"
#include "core/exception/invalid_argument_exception.h"
#include "core/exception/under_development_exception.h"
#include "sidemenu_item.h"

#include <QAbstractButton>
#include <QApplication>
#include <QButtonGroup>
#include <QEvent>
#include <QFrame>
#include <QLineEdit>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStyle>
#include <QVBoxLayout>

namespace {
QIcon themedIconWithFallback(const QStringList &names)
{
    for (const QString &name : names) {
        const QIcon icon = QIcon::fromTheme(name);
        if (!icon.isNull()) {
            return icon;
        }
    }

    return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
}
} // namespace

const QString Sidemenu::invalidSidemenuIDReason =
    QString("Sidemenu::ID must be in range (%1, %2)").arg(Sidemenu::ID_MIN).arg(Sidemenu::ID_MAX);

Sidemenu::Sidemenu(QWidget *parent) : QWidget(parent), buttonGroup(new QButtonGroup(this))
{
    const QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setSizePolicy(sizePolicy);
    setMinimumWidth(240);

    auto *const verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(4);
    verticalLayout->setContentsMargins(6, 6, 6, 6);

    m_searchBoxEdit = new QLineEdit(this);
    m_searchBoxEdit->setFocusPolicy(Qt::ClickFocus);
    verticalLayout->addWidget(m_searchBoxEdit);

    auto *const line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    verticalLayout->addWidget(line);

    auto *const scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    auto *const scrollAreaWidgetContents = new QWidget();
    m_scrollAreaLayout = new QVBoxLayout(scrollAreaWidgetContents);
    m_scrollAreaLayout->setSpacing(0);
    m_scrollAreaLayout->setContentsMargins(0, 0, 0, 0);
    scrollArea->setWidget(scrollAreaWidgetContents);
    verticalLayout->addWidget(scrollArea);

    buttonGroup->setExclusive(true);

    connect(buttonGroup, &QButtonGroup::idToggled, this, &Sidemenu::onButtonToggled);
    connect(m_searchBoxEdit, &QLineEdit::textChanged, this, &Sidemenu::onSearchTextChanged);

    // WIP: 適当なボタンを追加する
    registerItem(ID::IMAGE_ALL_IN_ONE);
    registerItem(ID::PHRASE_GENERATION);
    registerItem(ID::COMMAND_GENERATION);
    registerItem(ID::HTTP_REQUEST);
    registerItem(ID::DATA_CONVERSION);
    registerItem(ID::QR_CODE_GENERATION);
    registerItem(ID::DB_TOOL);

    m_scrollAreaLayout->addStretch();

    retranslateUi();
}

void Sidemenu::validateID(Sidemenu::ID id)
{
    const int intID = static_cast<int>(id);

    if (intID <= ID_MIN || ID_MAX <= intID) {
        throw InvalidArgumentException(intID, invalidSidemenuIDReason);
    }
}

QIcon Sidemenu::icon(Sidemenu::ID id)
{
    validateID(id);

    QStringList iconNames;
    switch (id) {
    case ID::HTTP_REQUEST:
        iconNames = {"network", "network-workgroup"};
        break;
    case ID::IMAGE_ALL_IN_ONE:
        iconNames = {"image-x-generic", "applications-graphics", "insert-image"};
        break;
    case ID::PHRASE_GENERATION:
        iconNames = {"library_books", "accessories-dictionary"};
        break;
    case ID::COMMAND_GENERATION:
        iconNames = {"terminal", "utilities-terminal"};
        break;
    case ID::DATA_CONVERSION:
        iconNames = {"question_mark", "view-refresh"};
        break;
    case ID::DB_TOOL:
        iconNames = {"database", "server-database"};
        break;
    case ID::QR_CODE_GENERATION:
        iconNames = {"qr_code", "insert-link"};
        break;

    default:
        throw UnderDevelopmentException();
    }

    return themedIconWithFallback(iconNames);
}

void Sidemenu::registerItem(ID id)
{
    auto *const item = new SidemenuItem(id, this);
    buttonGroup->addButton(item, static_cast<int>(id));
    m_scrollAreaLayout->addWidget(item);
    allItems.append(item);
}

void Sidemenu::selectItem(ID id)
{
    validateID(id);
    if (QAbstractButton *button = buttonGroup->button(static_cast<int>(id))) {
        button->setChecked(true);
    } else {
        qWarning() << "Sidemenu::selectItem - button not found for id" << static_cast<int>(id);
    }
}

void Sidemenu::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void Sidemenu::retranslateUi()
{
    m_searchBoxEdit->setPlaceholderText(tr("Search tools"));
}

void Sidemenu::onButtonToggled(int intID, bool checked)
{
    // NOTE: signal/slotでは例外を投げるべきではない
    try {
        const ID id = enum_cast<ID>(intID);

        validateID(id);

        if (checked) {
            emit itemSelected(id);
        }
    } catch (InvalidArgumentException<int> &e) {
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
