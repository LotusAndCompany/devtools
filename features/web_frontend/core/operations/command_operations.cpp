#include "command_operations.h"

#include "features/command/core/command_service.h"
#include "operation_support.h"

#include <QMetaType>
#include <QVariantList>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {

FrontendOperationService::Result commandData(const QVariantMap &payload)
{
    const QString categoryName = payload.value(QStringLiteral("category")).toString();
    CommandCategory category = CommandCategory::Unknown;
    if (categoryName == QStringLiteral("git")) {
        category = CommandCategory::Git;
    } else if (categoryName == QStringLiteral("docker")) {
        category = CommandCategory::Docker;
    } else if (categoryName == QStringLiteral("compose")) {
        category = CommandCategory::DockerCompose;
    } else {
        return invalidInput(QStringLiteral("Command category is not supported."));
    }
    const QList<CommandFunction> functions = CommandService::functions(category);
    if (payload.value(QStringLiteral("action")).toString() == QStringLiteral("list")) {
        QVariantList items;
        for (const CommandFunction &function : functions) {
            QVariantList options;
            for (const CommandOption &option : function.getOptions()) {
                options.append(QVariantMap{{QStringLiteral("name"), option.getName()},
                                           {QStringLiteral("required"), option.isRequired()},
                                           {QStringLiteral("title"), option.getTitle()},
                                           {QStringLiteral("quotes"), option.isRequiredQuotes()}});
            }
            items.append(QVariantMap{{QStringLiteral("name"), function.getName()},
                                     {QStringLiteral("description"), function.getDescription()},
                                     {QStringLiteral("options"), options}});
        }
        return {{{QStringLiteral("functions"), items}}, {}, {}, false};
    }
    bool functionIndexValid = false;
    bool optionIndexValid = false;
    const int functionIndex =
        payload.value(QStringLiteral("functionIndex")).toInt(&functionIndexValid);
    const int optionIndex = payload.value(QStringLiteral("optionIndex")).toInt(&optionIndexValid);
    if (!functionIndexValid || !optionIndexValid) {
        return invalidInput(QStringLiteral("Select a valid command and option."));
    }
    QString value;
    if (!readText(payload, QStringLiteral("value"), &value)) {
        return invalidInput(QStringLiteral("Command value must be a UTF-8 string."));
    }
    const CommandGenerationResult result =
        CommandService::generate(category, functionIndex, optionIndex, value);
    if (result.error != CommandGenerationError::None) {
        return invalidInput(result.error == CommandGenerationError::InvalidValue
                                ? QStringLiteral("Quotes are not allowed in command values.")
                                : QStringLiteral("Select a command and option."));
    }
    return {{{QStringLiteral("command"), result.command}}, {}, {}, false};
}

} // namespace FrontendOperationHandlers
