#include "command_service.h"

#include <QCoreApplication>

#include <algorithm>
#include <utility>

namespace {
QString translate(const char *source)
{
    return QCoreApplication::translate("Command", source);
}

template <typename... Rest>
QList<CommandOption> makeOptions(CommandOption first, Rest... rest)
{
    QList<CommandOption> options;
    options.append(std::move(first));
    (options.append(std::move(rest)), ...);
    return options;
}

QList<CommandFunction> gitCommands()
{
    return {
        CommandFunction(
            "git add", translate("Add changes"),
            makeOptions(CommandOption(translate("None"), true, translate("Path"), false))),
        CommandFunction("git commit", translate("Create a commit"),
                        makeOptions(CommandOption(translate("None"), false, "", false),
                                    CommandOption("-m", true, translate("Message"), true),
                                    CommandOption("-a", false, "", false))),
        CommandFunction("git log", translate("Show commit history"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("git status", translate("Show branch status"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction(
            "git diff", translate("Show differences"),
            makeOptions(CommandOption(translate("None"), true, translate("Target"), false))),
        CommandFunction("git merge", translate("Merge"),
                        makeOptions(CommandOption(translate("None"), false, "", false),
                                    CommandOption("--abort", false, "", false))),
        CommandFunction(
            "git remote", translate("Remote"),
            makeOptions(CommandOption("-v", false, "", false),
                        CommandOption("set-url", true, translate("Remote name and URL"), false))),
    };
}

QList<CommandFunction> dockerCommands()
{
    return {
        CommandFunction("docker build", translate("Build"),
                        makeOptions(CommandOption(translate("None"), true, "", false),
                                    CommandOption("-t", true, translate("Name and tag"), false),
                                    CommandOption("-f", true, translate("File"), false))),
        CommandFunction("docker images", translate("List images"),
                        makeOptions(CommandOption(translate("None"), true, "", false),
                                    CommandOption("--all", false, "", false),
                                    CommandOption("--quiet", false, "", false))),
        CommandFunction("docker run", translate("Create and run a container"),
                        makeOptions(CommandOption(translate("None"), true, "", false))),
        CommandFunction("docker image prune", translate("Remove unused images"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker exec", translate("Run a command in a running container"),
                        makeOptions(CommandOption(translate("None"), true, "", false))),
        CommandFunction("docker pull", translate("Download an image from a repository"),
                        makeOptions(CommandOption(translate("None"), true, "", false))),
        CommandFunction("docker push", translate("Push an image to a repository"),
                        makeOptions(CommandOption(translate("None"), true, "", false))),
        CommandFunction(
            "docker logs", translate("Show logs"),
            makeOptions(CommandOption(translate("None"), true, translate("Container"), false))),
        CommandFunction(
            "docker stop", translate("Stop a container"),
            makeOptions(CommandOption(translate("None"), true, translate("Container"), false))),
        CommandFunction("docker rm", translate("Remove a stopped container"),
                        makeOptions(CommandOption(translate("None"), true, "", false))),
        CommandFunction("docker rmi", translate("Remove an image"),
                        makeOptions(CommandOption(translate("None"), true, "", false))),
        CommandFunction("docker network ls", translate("List created networks"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker volume ls", translate("List created volumes"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
    };
}

QList<CommandFunction> dockerComposeCommands()
{
    return {
        CommandFunction("docker compose build", translate("Build services"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker compose exec", translate("Run a command in a running container"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker compose up", translate("Start containers"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker compose down",
                        translate("Stop containers and remove related resources"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker compose restart", translate("Restart containers"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker compose stop", translate("Stop containers"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker compose logs", translate("Show container logs"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker compose ps", translate("Show container status"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker compose start", translate("Start stopped services"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
        CommandFunction("docker compose kill", translate("Force stop containers"),
                        makeOptions(CommandOption(translate("None"), false, "", false))),
    };
}
} // namespace

QList<CommandFunction> CommandService::functions(CommandCategory category)
{
    switch (category) {
    case CommandCategory::Git:
        return gitCommands();
    case CommandCategory::Docker:
        return dockerCommands();
    case CommandCategory::DockerCompose:
        return dockerComposeCommands();
    case CommandCategory::Unknown:
        return {};
    }
    return {};
}

CommandGenerationResult CommandService::generate(CommandCategory category, int functionIndex,
                                                 int optionIndex, const QString &value)
{
    const QList<CommandFunction> command_list = functions(category);
    if (functionIndex < 0 || functionIndex >= command_list.size()) {
        return {{}, CommandGenerationError::InvalidSelection};
    }

    const CommandFunction &function = command_list[functionIndex];
    const QList<CommandOption> option_list = function.getOptions();
    if (optionIndex < 0 || optionIndex >= option_list.size()) {
        return {{}, CommandGenerationError::InvalidSelection};
    }

    const CommandOption &option = option_list[optionIndex];
    if (!containsNoQuotes(value)) {
        return {{}, CommandGenerationError::InvalidValue};
    }

    QString command = function.getName();
    if (option.getName() != translate("None")) {
        command += " " + option.getName();
    }
    if (option.isRequired()) {
        if (option.isRequiredQuotes()) {
            command += " \"" + value + "\"";
        } else {
            command += " " + value;
        }
    }
    return {command, CommandGenerationError::None};
}

bool CommandService::containsNoQuotes(const QString &value)
{
    static const QString quotes = "\"'`";
    return std::all_of(quotes.cbegin(), quotes.cend(),
                       [&value](const QChar &quote) { return !value.contains(quote); });
}
