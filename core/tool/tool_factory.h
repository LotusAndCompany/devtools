#ifndef TOOL_FACTORY_H
#define TOOL_FACTORY_H

#include <QObject>
#include "tool.h"

class ToolFactory : public QObject
{
    Q_OBJECT
public:
    ToolFactory(QObject *parent = nullptr);

    static QString toolName(Tool::ID id) noexcept(false);
    static QString toolDescription(Tool::ID id) noexcept(false);

protected:
    static const QString invalidToolIDReason;
    static Tool *createTool(Tool::ID id) noexcept(false);
};

#endif // TOOL_FACTORY_H
