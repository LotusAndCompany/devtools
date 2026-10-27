#ifndef COMMAND_SERVICE_H
#define COMMAND_SERVICE_H

#include "command_function.h"

#include <QList>
#include <QString>

#include <cstdint>

enum class CommandCategory : uint8_t { Unknown = 0, Git = 1, Docker = 2, DockerCompose = 3 };

enum class CommandGenerationError : uint8_t { None, InvalidSelection, InvalidValue };

struct CommandGenerationResult
{
    QString command;
    CommandGenerationError error{CommandGenerationError::None};
};

class CommandService
{
public:
    static QList<CommandFunction> functions(CommandCategory category);
    static CommandGenerationResult generate(CommandCategory category, int functionIndex,
                                            int optionIndex, const QString &value);
    static bool containsNoQuotes(const QString &value);
};

#endif // COMMAND_SERVICE_H
