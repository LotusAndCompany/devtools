#ifndef COMMAND_FUNCTION_H
#define COMMAND_FUNCTION_H
#pragma once
#include "command_option.h"

#include <QList>
#include <QString>

#include <utility>

class CommandFunction
{
public:
    CommandFunction(QString name, QString description, const QList<CommandOption> &options)
        : name(std::move(name)), description(std::move(description)), options(options)
    {}

    [[nodiscard]] QString getName() const { return name; }
    [[nodiscard]] QString getDescription() const { return description; }
    [[nodiscard]] QList<CommandOption> getOptions() const { return options; }

private:
    QString name;
    QString description;
    QList<CommandOption> options;
};
#endif // COMMAND_FUNCTION_H
