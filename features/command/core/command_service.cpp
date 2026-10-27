#include "command_service.h"

#include <QCoreApplication>

namespace {
QString translate(const char *source)
{
    return QCoreApplication::translate("Command", source);
}

QList<CommandFunction> gitCommands()
{
    return {
        CommandFunction("git add", translate("Add changes"),
                        {CommandOption(translate("None"), true, translate("Path"), false)}),
        CommandFunction("git commit", translate("Create a commit"),
                        {CommandOption(translate("None"), false, "", false),
                         CommandOption("-m", true, translate("Message"), true),
                         CommandOption("-a", false, "", false)}),
        CommandFunction("git log", translate("Show commit history"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("git status", translate("Show branch status"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("git diff", translate("Show differences"),
                        {CommandOption(translate("None"), true, translate("Target"), false)}),
        CommandFunction("git merge", translate("Merge"),
                        {CommandOption(translate("None"), false, "", false),
                         CommandOption("--abort", false, "", false)}),
        CommandFunction("git remote", translate("Remote"),
                        {CommandOption("-v", false, "", false),
                         CommandOption("set-url", true, translate("Remote name and URL"), false)}),
    };
}

QList<CommandFunction> dockerCommands()
{
    return {
        CommandFunction("docker build", translate("Build"),
                        {CommandOption(translate("None"), true, "", false),
                         CommandOption("-t", true, translate("Name and tag"), false),
                         CommandOption("-f", true, translate("File"), false)}),
        CommandFunction(
            "docker images", translate("List images"),
            {CommandOption(translate("None"), true, "", false),
             CommandOption("--all", false, "", false), CommandOption("--quiet", false, "", false)}),
        CommandFunction("docker run", translate("Create and run a container"),
                        {CommandOption(translate("None"), true, "", false)}),
        CommandFunction("docker image prune", translate("Remove unused images"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker exec", translate("Run a command in a running container"),
                        {CommandOption(translate("None"), true, "", false)}),
        CommandFunction("docker pull", translate("Download an image from a repository"),
                        {CommandOption(translate("None"), true, "", false)}),
        CommandFunction("docker push", translate("Push an image to a repository"),
                        {CommandOption(translate("None"), true, "", false)}),
        CommandFunction("docker logs", translate("Show logs"),
                        {CommandOption(translate("None"), true, translate("Container"), false)}),
        CommandFunction("docker stop", translate("Stop a container"),
                        {CommandOption(translate("None"), true, translate("Container"), false)}),
        CommandFunction("docker rm", translate("Remove a stopped container"),
                        {CommandOption(translate("None"), true, "", false)}),
        CommandFunction("docker rmi", translate("Remove an image"),
                        {CommandOption(translate("None"), true, "", false)}),
        CommandFunction("docker network ls", translate("List created networks"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker volume ls", translate("List created volumes"),
                        {CommandOption(translate("None"), false, "", false)}),
    };
}

QList<CommandFunction> dockerComposeCommands()
{
    return {
        CommandFunction("docker compose build", translate("Build services"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker compose exec", translate("Run a command in a running container"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker compose up", translate("Start containers"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker compose down",
                        translate("Stop containers and remove related resources"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker compose restart", translate("Restart containers"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker compose stop", translate("Stop containers"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker compose logs", translate("Show container logs"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker compose ps", translate("Show container status"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker compose start", translate("Start stopped services"),
                        {CommandOption(translate("None"), false, "", false)}),
        CommandFunction("docker compose kill", translate("Force stop containers"),
                        {CommandOption(translate("None"), false, "", false)}),
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
    for (const QChar &quote : quotes) {
        if (value.contains(quote)) {
            return false;
        }
    }
    return true;
}
