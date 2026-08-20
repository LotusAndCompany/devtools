#include "command.h"

#include "command_function.h"
#include "features/framework/gui/design_system.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

Command::Command(QWidget *parent) : QWidget(parent)
{
    buildUi();
    init();

    connect(category_list, &QComboBox::currentIndexChanged, this, &Command::selectedCategory);
    connect(functions_list, &QComboBox::currentIndexChanged, this, &Command::selectedFunction);
    connect(option_list, &QComboBox::currentIndexChanged, this, &Command::selectedOption);
    connect(reset_button, &QPushButton::clicked, this, &Command::reset);
    connect(generate_button, &QPushButton::clicked, this, &Command::generate);
    connect(clear_button, &QPushButton::clicked, this, &Command::clear);
    connect(copy_button, &QPushButton::clicked, this, &Command::copy);
}

// NOLINTNEXTLINE(readability-function-size)
void Command::buildUi()
{
    setObjectName(QStringLiteral("Command"));

    auto *const layout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(layout);

    input_heading = DevTools::Ui::createPaneHeading(tr("Input"), this);
    layout->addWidget(input_heading);

    auto *const inputPane = new QGroupBox(this);
    inputPane->setObjectName(QStringLiteral("inputPane"));
    auto *const inputLayout = new QGridLayout(inputPane);
    DevTools::Ui::applyPanelLayout(inputLayout);

    category_list = new QComboBox(inputPane);
    category_list->setObjectName(QStringLiteral("categoryList"));
    inputLayout->addWidget(category_list, 0, 0, 1, 2);

    functions_label = new QLabel(tr("Functions List"), inputPane);
    functions_label->setObjectName(QStringLiteral("functionsLabel"));
    inputLayout->addWidget(functions_label, 1, 0);

    functions_list = new QComboBox(inputPane);
    functions_list->setObjectName(QStringLiteral("functionsList"));
    functions_list->setEnabled(false);
    functions_list->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    inputLayout->addWidget(functions_list, 2, 0, 1, 2);

    option_label = new QLabel(tr("Option List"), inputPane);
    option_label->setObjectName(QStringLiteral("optionLabel"));
    inputLayout->addWidget(option_label, 3, 0);

    option_list = new QComboBox(inputPane);
    option_list->setObjectName(QStringLiteral("optionList"));
    option_list->setEnabled(false);
    inputLayout->addWidget(option_list, 4, 0, 1, 2);

    text_label = new QLabel(tr("TextLabel"), inputPane);
    text_label->setObjectName(QStringLiteral("label"));
    inputLayout->addWidget(text_label, 5, 0);

    text_edit = new QLineEdit(inputPane);
    text_edit->setObjectName(QStringLiteral("textEdit"));
    inputLayout->addWidget(text_edit, 6, 0, 1, 2);

    reset_button = new QPushButton(tr("Reset"), inputPane);
    reset_button->setObjectName(QStringLiteral("resetButton"));
    DevTools::Ui::configureCompactButton(reset_button);

    generate_button = new QPushButton(tr("Generate command"), inputPane);
    generate_button->setObjectName(QStringLiteral("generateButton"));
    DevTools::Ui::configureCompactButton(generate_button);

    auto *actionLayout = new QHBoxLayout;
    actionLayout->addWidget(reset_button);
    actionLayout->addWidget(generate_button);
    DevTools::Ui::configureActionBar(actionLayout, DevTools::Ui::ActionBarAlignment::Trailing);
    inputLayout->addLayout(actionLayout, 7, 0, 1, 2);

    layout->addWidget(inputPane);

    output_heading = DevTools::Ui::createPaneHeading(tr("Generated Result"), this);
    layout->addWidget(output_heading);

    auto *const outputPane = new QGroupBox(this);
    outputPane->setObjectName(QStringLiteral("outputPane"));
    auto *const outputLayout = new QGridLayout(outputPane);
    DevTools::Ui::applyPanelLayout(outputLayout);

    text_browser = new QPlainTextEdit(outputPane);
    text_browser->setObjectName(QStringLiteral("textBrowser"));
    DevTools::Ui::configureCodeEditor(text_browser);
    text_browser->setReadOnly(true);
    outputLayout->addWidget(text_browser, 0, 0, 1, 2);
    outputLayout->setRowStretch(0, 1);

    copy_button = new QPushButton(tr("Copy"), outputPane);
    copy_button->setObjectName(QStringLiteral("copyButton"));
    DevTools::Ui::configureCompactButton(copy_button);

    clear_button = new QPushButton(tr("Clear"), outputPane);
    clear_button->setObjectName(QStringLiteral("clearButton"));
    DevTools::Ui::configureCompactButton(clear_button);

    auto *outputActionLayout = new QHBoxLayout;
    outputActionLayout->addWidget(copy_button);
    outputActionLayout->addWidget(clear_button);
    DevTools::Ui::configureActionBar(outputActionLayout,
                                     DevTools::Ui::ActionBarAlignment::Trailing);
    outputLayout->addLayout(outputActionLayout, 1, 0, 1, 2);

    layout->addWidget(outputPane);
    layout->setStretch(3, 1);
}

void Command::init()
{
    const QStringList category_items{tr("Categories"), tr("1: Git commands"),
                                     tr("2: Docker commands"), tr("3: Docker Compose commands")};

    category_list->addItems(category_items);

    text_label->setVisible(false);
    text_edit->setVisible(false);

    functions_label->setVisible(false);
    functions_list->setVisible(false);
    option_label->setVisible(false);
    option_list->setVisible(false);
}

QList<CommandFunction> getGitCommands()
{
    return {
        CommandFunction("git add", Command::tr("Add changes"),
                        {
                            CommandOption(Command::tr("None"), true, Command::tr("Path"), false),
                        }),
        CommandFunction("git commit", Command::tr("Create a commit"),
                        {CommandOption(Command::tr("None"), false, "", false),
                         CommandOption("-m", true, Command::tr("Message"), true),
                         CommandOption("-a", false, "", false)}),
        CommandFunction("git log", Command::tr("Show commit history"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("git status", Command::tr("Show branch status"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("git diff", Command::tr("Show differences"),
                        {
                            CommandOption(Command::tr("None"), true, Command::tr("Target"), false),
                        }),
        CommandFunction("git merge", Command::tr("Merge"),
                        {CommandOption(Command::tr("None"), false, "", false),
                         CommandOption("--abort", false, "", false)}),
        CommandFunction(
            "git remote", Command::tr("Remote"),
            {CommandOption("-v", false, "", false),
             CommandOption("set-url", true, Command::tr("Remote name and URL"), false)})};
}

QList<CommandFunction> getDockerCommands()
{
    return {
        CommandFunction("docker build", Command::tr("Build"),
                        {
                            CommandOption(Command::tr("None"), true, "", false),
                            CommandOption("-t", true, Command::tr("Name and tag"), false),
                            CommandOption("-f", true, Command::tr("File"), false),
                        }),
        CommandFunction("docker images", Command::tr("List images"),
                        {
                            CommandOption(Command::tr("None"), true, "", false),
                            CommandOption("--all", false, "", false),
                            CommandOption("--quiet", false, "", false),
                        }),
        CommandFunction("docker run", Command::tr("Create and run a container"),
                        {
                            CommandOption(Command::tr("None"), true, "", false),
                        }),
        CommandFunction("docker image prune", Command::tr("Remove unused images"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker exec", Command::tr("Run a command in a running container"),
                        {
                            CommandOption(Command::tr("None"), true, "", false),
                        }),
        CommandFunction("docker pull", Command::tr("Download an image from a repository"),
                        {
                            CommandOption(Command::tr("None"), true, "", false),
                        }),
        CommandFunction("docker push", Command::tr("Push an image to a repository"),
                        {
                            CommandOption(Command::tr("None"), true, "", false),
                        }),
        CommandFunction(
            "docker logs", Command::tr("Show logs"),
            {
                CommandOption(Command::tr("None"), true, Command::tr("Container"), false),
            }),
        CommandFunction(
            "docker stop", Command::tr("Stop a container"),
            {
                CommandOption(Command::tr("None"), true, Command::tr("Container"), false),
            }),
        CommandFunction("docker rm", Command::tr("Remove a stopped container"),
                        {
                            CommandOption(Command::tr("None"), true, "", false),
                        }),
        CommandFunction("docker rmi", Command::tr("Remove an image"),
                        {
                            CommandOption(Command::tr("None"), true, "", false),
                        }),
        CommandFunction("docker network ls", Command::tr("List created networks"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker volume ls", Command::tr("List created volumes"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
    };
}

QList<CommandFunction> getDockerComposeCommands()
{
    return {
        CommandFunction("docker compose build", Command::tr("Build services"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker compose exec", Command::tr("Run a command in a running container"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker compose up", Command::tr("Start containers"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker compose down",
                        Command::tr("Stop containers and remove related resources"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker compose restart", Command::tr("Restart containers"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker compose stop", Command::tr("Stop containers"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker compose logs", Command::tr("Show container logs"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker compose ps", Command::tr("Show container status"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker compose start", Command::tr("Start stopped services"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
        CommandFunction("docker compose kill", Command::tr("Force stop containers"),
                        {
                            CommandOption(Command::tr("None"), false, "", false),
                        }),
    };
}

void Command::selectedCategory()
{
    const int selectedIndex = category_list->currentIndex();

    functions_list->clear();

    QList<CommandFunction> commandList;
    QStringList commandNames;

    switch (selectedIndex) {
    case 1:
        commandList = getGitCommands();
        for (const CommandFunction &cmd : std::as_const(commandList)) {
            commandNames.append(cmd.getName() + " " + cmd.getDescription());
        }
        functions_list->addItems(commandNames);
        functions_label->setVisible(true);
        functions_list->setVisible(true);
        functions_list->setEnabled(true);
        break;
    case 2:
        commandList = getDockerCommands();
        for (const CommandFunction &cmd : std::as_const(commandList)) {
            commandNames.append(cmd.getName() + " " + cmd.getDescription());
        }
        functions_list->addItems(commandNames);
        functions_label->setVisible(true);
        functions_list->setVisible(true);
        functions_list->setEnabled(true);
        break;
    case 3:
        commandList = getDockerComposeCommands();
        for (const CommandFunction &cmd : std::as_const(commandList)) {
            commandNames.append(cmd.getName() + " " + cmd.getDescription());
        }
        functions_list->addItems(commandNames);
        functions_label->setVisible(true);
        functions_list->setVisible(true);
        functions_list->setEnabled(true);
        break;
    default:
        reset();
        break;
    }
}

void Command::selectedFunction()
{
    text_edit->clear();
    option_list->clear();
    option_list->setVisible(false);
    option_list->setEnabled(false);

    const int selectedCategoryIndex = category_list->currentIndex();
    const int selectedFunctionsIndex = functions_list->currentIndex();

    QList<CommandFunction> commandList;

    switch (selectedCategoryIndex) {
    case 1:
        commandList = getGitCommands();
        break;
    case 2:
        commandList = getDockerCommands();
        break;
    case 3:
        commandList = getDockerComposeCommands();
        break;
    default:
        break;
    }

    if (selectedFunctionsIndex < 0 || selectedFunctionsIndex >= commandList.size()) {
        return;
    }
    QList<CommandOption> const optionList = commandList[selectedFunctionsIndex].getOptions();

    if (static_cast<int>(!optionList.empty()) != 0) {
        text_edit->clear();
        text_label->setVisible(false);
        text_edit->setVisible(false);
        QStringList optionNames;

        for (const CommandOption &opt : optionList) {
            optionNames.append(opt.getName());
        }
        option_list->addItems(optionNames);
        option_label->setVisible(true);
        option_list->setVisible(true);
        option_list->setEnabled(true);
    }
}

void Command::selectedOption()
{
    const int selectedCategoryIndex = category_list->currentIndex();
    const int selectedFunctionsIndex = functions_list->currentIndex();
    const int selectedOptionIndex = option_list->currentIndex();

    QList<CommandFunction> commandList;

    switch (selectedCategoryIndex) {
    case 1:
        commandList = getGitCommands();
        break;
    case 2:
        commandList = getDockerCommands();
        break;
    case 3:
        commandList = getDockerComposeCommands();
        break;
    default:
        break;
    }
    if (selectedFunctionsIndex < 0 || selectedFunctionsIndex >= commandList.size()) {
        return;
    }

    QList<CommandOption> optionList = commandList[selectedFunctionsIndex].getOptions();
    if (selectedOptionIndex < 0 || selectedOptionIndex >= optionList.size()) {
        return;
    }

    const CommandOption &option = optionList[selectedOptionIndex];

    if (option.isRequired()) {
        text_label->setText(option.getTitle());
        text_label->setVisible(true);
        text_edit->setVisible(true);
    } else {
        text_edit->clear();
        text_label->setVisible(false);
        text_edit->setVisible(false);
    }
}

void Command::reset()
{
    category_list->setCurrentIndex(0);

    functions_list->clear();
    option_list->clear();

    text_edit->clear();

    text_label->setVisible(false);
    text_edit->setVisible(false);

    functions_label->setVisible(false);
    functions_list->setVisible(false);
    functions_list->setEnabled(false);

    option_label->setVisible(false);
    option_list->setVisible(false);
    option_list->setEnabled(false);
}

void Command::clear()
{
    text_browser->clear();
}

bool containsNoQuotes(const QString &str)
{
    QString const quotes = "\"'`";

    // NOLINTNEXTLINE(readability-use-anyofallof)
    for (const QChar &quote : quotes) {
        if (str.contains(quote)) {
            return false;
        }
    }
    return true;
}

void showErrorAlert()
{
    QMessageBox::critical(nullptr, Command::tr("Error"),
                          Command::tr("This value is invalid. Please check and try again."));
}

void Command::generate()
{
    const int selectedCategoryIndex = category_list->currentIndex();
    const int selectedFunctionsIndex = functions_list->currentIndex();
    const int selectedOptionIndex = option_list->currentIndex();

    QList<CommandFunction> commandList;
    switch (selectedCategoryIndex) {
    case 1:
        commandList = getGitCommands();
        break;
    case 2:
        commandList = getDockerCommands();
        break;
    case 3:
        commandList = getDockerComposeCommands();
        break;
    default:
        break;
    }
    if (selectedFunctionsIndex < 0 || selectedFunctionsIndex >= commandList.size()) {
        return;
    }

    QList<CommandOption> optionList = commandList[selectedFunctionsIndex].getOptions();
    if (selectedOptionIndex < 0 || selectedOptionIndex >= optionList.size()) {
        return;
    }

    const CommandOption &option = optionList[selectedOptionIndex];

    const QString value1 = text_edit->text();

    if (!containsNoQuotes(value1)) {
        showErrorAlert();
    } else {
        QString command = commandList[selectedFunctionsIndex].getName();
        if (option_list->isEnabled() && option.getName() != tr("None")) {
            command += " " + option.getName();
        }
        if (option.isRequired() || text_edit->isVisible()) {
            if (option.isRequiredQuotes()) {
                command += " \"" + value1 + "\"";
            } else {
                command += " " + value1;
            }
        }
        text_browser->setPlainText(command);
    }
}

void Command::copy()
{
    const QString text = text_browser->toPlainText();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
