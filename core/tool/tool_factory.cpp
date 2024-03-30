#include "tool_factory.h"
#include "core/exception/invalid_argument_exception.h"

const QString ToolFactory::invalidToolIDReason
    = QString("Tool id must be in range (%d, %d)").arg(Tool::ID_UNDEFINED).arg(Tool::ID_MAX);

ToolFactory::ToolFactory(QObject *parent)
    : QObject(parent)
{}

Tool *ToolFactory::createTool(Tool::ID id)
{
    // TODO: return new SomeTool();
    switch (id) {
    default:
        throw InvalidArgumentException(static_cast<int>(id), invalidToolIDReason);
    }
}

QString ToolFactory::toolName(Tool::ID id)
{
    // TODO: return tr("Tool Name");
    switch (id) {
    default:
        throw InvalidArgumentException(static_cast<int>(id), invalidToolIDReason);
    }
}

QString ToolFactory::toolDescription(Tool::ID id)
{
    // TODO: return tr("Tool description");
    switch (id) {
    default:
        throw InvalidArgumentException(static_cast<int>(id), invalidToolIDReason);
    }
}
