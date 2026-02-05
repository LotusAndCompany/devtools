#ifndef COMMAND_OPTION_H
#define COMMAND_OPTION_H
#pragma once
#include <QString>

#include <utility>

class CommandOption
{
public:
    CommandOption(QString name, bool required, QString title, bool requiredQuotes)
        : name(std::move(name))
        , required(required)
        , title(std::move(title))
        , requiredQuotes(requiredQuotes)
    {}

    [[nodiscard]] QString getName() const { return name; }
    [[nodiscard]] bool isRequired() const { return required; }
    [[nodiscard]] QString getTitle() const { return title; }
    [[nodiscard]] bool isRequiredQuotes() const { return requiredQuotes; }

private:
    QString name;
    bool required;
    QString title;
    bool requiredQuotes;
};

#endif // COMMAND_OPTION_H
