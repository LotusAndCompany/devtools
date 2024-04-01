#include "tool.h"
#include <QEvent>
#include "core/exception/invalid_argument_exception.h"
#include "core/exception/under_development_exception.h"

const QString Tool::invalidToolIDReason
    = QString("Tool::ID must be in range (%1, %2)").arg(Tool::ID_UNDEFINED).arg(Tool::ID_MAX);

Tool::Tool(Tool::ID id, const QString &stringID, QObject *parent)
    : QObject(parent)
    , id(id)
    , stringID(stringID)
    , _translatable(translatable(id))
{}

void Tool::validateID(ID id)
{
    const int intID = static_cast<int>(id);

    if (intID <= ID_UNDEFINED || ID_MAX <= intID)
        throw InvalidArgumentException(intID, invalidToolIDReason);
}

const Tool::Translatable Tool::translatable(ID id)
{
    validateID(id);

    // TODO: return Translatable{tr("Tool Name"), tr("Tool description")}
    switch (id) {
    default:
        throw UnderDevelopmentException();
    }
}

bool Tool::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        QObject::event(event);
        _translatable = translatable(id);
        return true;
    default:
        return QObject::event(event);
    }
}
