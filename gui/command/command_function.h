#ifndef COMMAND_FUNCTION_H
#define COMMAND_FUNCTION_H
#pragma once
#include "command_option.h"

#include <QList>
#include <QString>

class CommandFunction
{
public:
    CommandFunction(const QString &name, const QString &description,
                    const QList<CommandOption> &options)
        : name(name), description(description), options(options)
    {}

    QString getName() const { return name; }
    QString getDescription() const { return description; }
    QList<CommandOption> getOptions() const { return options; }

private:
    QString name;
    QString description;
    QList<CommandOption> options;
};
#endif // COMMAND_FUNCTION_H
