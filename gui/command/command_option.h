#ifndef COMMAND_OPTION_H
#define COMMAND_OPTION_H
#pragma once
#include <QString>

class CommandOption {
public:
    CommandOption(const QString& name, bool required, const QString& title, bool requiredQuotes)
        : name(name), required(required), title(title), requiredQuotes(requiredQuotes){}

    QString getName() const { return name; }
    bool isRequired() const { return required; }
    QString getTitle() const { return title; }
    bool isRequiredQuotes() const { return requiredQuotes; }
private:
    QString name;
    bool required;
    QString title;
    bool requiredQuotes;
};


#endif // COMMAND_OPTION_H
