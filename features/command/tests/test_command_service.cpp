#include "features/command/core/command_service.h"

#include <QtTest>

class TestCommandService : public QObject
{
    Q_OBJECT

private slots:
    void listsCommands();
    void generatesCommands();
    void rejectsQuotedValues();
    void rejectsInvalidSelections();
};

void TestCommandService::listsCommands()
{
    const auto git = CommandService::functions(CommandCategory::Git);
    const auto docker = CommandService::functions(CommandCategory::Docker);
    const auto compose = CommandService::functions(CommandCategory::DockerCompose);

    QCOMPARE(git.size(), 7);
    QCOMPARE(docker.size(), 13);
    QCOMPARE(compose.size(), 10);
    QCOMPARE(git[0].getName(), QString("git add"));
    QCOMPARE(git[1].getDescription(), QString("Create a commit"));
    QCOMPARE(docker[0].getOptions().size(), 3);
    QVERIFY(CommandService::functions(CommandCategory::Unknown).isEmpty());
}

void TestCommandService::generatesCommands()
{
    auto result = CommandService::generate(CommandCategory::Git, 2, 0, "ignored");
    QCOMPARE(result.error, CommandGenerationError::None);
    QCOMPARE(result.command, QString("git log"));

    result = CommandService::generate(CommandCategory::Git, 1, 1, "release candidate");
    QCOMPARE(result.error, CommandGenerationError::None);
    QCOMPARE(result.command, QString("git commit -m \"release candidate\""));

    result = CommandService::generate(CommandCategory::Git, 1, 2, "ignored");
    QCOMPARE(result.error, CommandGenerationError::None);
    QCOMPARE(result.command, QString("git commit -a"));

    result = CommandService::generate(CommandCategory::Docker, 2, 0, "container");
    QCOMPARE(result.error, CommandGenerationError::None);
    QCOMPARE(result.command, QString("docker run container"));
}

void TestCommandService::rejectsQuotedValues()
{
    QVERIFY(CommandService::containsNoQuotes("plain"));
    QVERIFY(!CommandService::containsNoQuotes("double\"quote"));
    QVERIFY(!CommandService::containsNoQuotes("single'quote"));
    QVERIFY(!CommandService::containsNoQuotes("back`tick"));

    const auto result = CommandService::generate(CommandCategory::Git, 1, 1, "bad\"value");
    QCOMPARE(result.error, CommandGenerationError::InvalidValue);
    QVERIFY(result.command.isEmpty());
}

void TestCommandService::rejectsInvalidSelections()
{
    const auto invalid_category =
        CommandService::generate(CommandCategory::Unknown, 0, 0, QString());
    const auto invalid_function = CommandService::generate(CommandCategory::Git, -1, 0, QString());
    const auto invalid_option = CommandService::generate(CommandCategory::Git, 0, 99, QString());

    QCOMPARE(invalid_category.error, CommandGenerationError::InvalidSelection);
    QCOMPARE(invalid_function.error, CommandGenerationError::InvalidSelection);
    QCOMPARE(invalid_option.error, CommandGenerationError::InvalidSelection);
}

QTEST_GUILESS_MAIN(TestCommandService)
#include "test_command_service.moc"
