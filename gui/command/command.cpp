#include "command.h"

#include "command_function.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFontMetrics>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextBrowser>

#include <algorithm>

Command::Command(QWidget *parent) : QGroupBox(parent)
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

void Command::buildUi()
{
    setObjectName(QStringLiteral("Command"));
    setWindowTitle(tr("GroupBox"));
    resize(699, 711);

    auto *layout = new QGridLayout(this);
    layout->setObjectName(QStringLiteral("gridLayout"));

    category_list = new QComboBox(this);
    category_list->setObjectName(QStringLiteral("categoryList"));
    layout->addWidget(category_list, 0, 0, 1, 2);

    functions_label = new QLabel(tr("Functions List"), this);
    functions_label->setObjectName(QStringLiteral("functionsLabel"));
    layout->addWidget(functions_label, 1, 0);

    functions_list = new QComboBox(this);
    functions_list->setObjectName(QStringLiteral("functionsList"));
    functions_list->setEnabled(false);
    functions_list->setAutoFillBackground(false);
    layout->addWidget(functions_list, 2, 0, 1, 2);

    option_label = new QLabel(tr("Option List"), this);
    option_label->setObjectName(QStringLiteral("optionLabel"));
    layout->addWidget(option_label, 3, 0);

    option_list = new QComboBox(this);
    option_list->setObjectName(QStringLiteral("optionList"));
    option_list->setEnabled(false);
    layout->addWidget(option_list, 4, 0, 1, 2);

    text_label = new QLabel(tr("TextLabel"), this);
    text_label->setObjectName(QStringLiteral("label"));
    layout->addWidget(text_label, 5, 0);

    text_edit = new QLineEdit(this);
    text_edit->setObjectName(QStringLiteral("textEdit"));
    layout->addWidget(text_edit, 6, 0, 1, 2);

    reset_button = new QPushButton(tr("Reset"), this);
    reset_button->setObjectName(QStringLiteral("resetButton"));
    reset_button->setFont(QFont(QStringLiteral(".AppleSystemUIFont")));
    layout->addWidget(reset_button, 7, 0);

    generate_button = new QPushButton(tr("Generate command"), this);
    generate_button->setObjectName(QStringLiteral("generateButton"));
    layout->addWidget(generate_button, 7, 1);

    text_browser = new QTextBrowser(this);
    text_browser->setObjectName(QStringLiteral("textBrowser"));
    layout->addWidget(text_browser, 8, 0, 1, 4);

    copy_button = new QPushButton(tr("Copy"), this);
    copy_button->setObjectName(QStringLiteral("copyButton"));
    layout->addWidget(copy_button, 9, 2);

    clear_button = new QPushButton(tr("Clear"), this);
    clear_button->setObjectName(QStringLiteral("clearButton"));
    layout->addWidget(clear_button, 9, 3);
}

void Command::init()
{
    const QStringList category_items{"カテゴリ一覧", "1: Gitコマンド", "2: Dockerコマンド",
                                     "3: DockerComposeコマンド"};

    category_list->addItems(category_items);

    adjustCommandBoxWidth();

    text_label->setVisible(false);
    text_edit->setVisible(false);

    functions_label->setVisible(false);
    functions_list->setVisible(false);
    option_label->setVisible(false);
    option_list->setVisible(false);
}

QList<CommandFunction> getGitCommands()
{
    return {CommandFunction("git add", "変更を追加",
                            {
                                CommandOption("なし", true, "Path", false),
                            }),
            CommandFunction("git commit", "コミットを作成",
                            {CommandOption("なし", false, "", false),
                             CommandOption("-m", true, "メッセージ", true),
                             CommandOption("-a", false, "", false)}),
            CommandFunction("git log", "コミット履歴を確認",
                            {
                                CommandOption("なし", false, "", false),
                            }),
            CommandFunction("git status", "ブランチの状態確認",
                            {
                                CommandOption("なし", false, "", false),
                            }),
            CommandFunction("git diff", "差分を確認",
                            {
                                CommandOption("なし", true, "比較対象", false),
                            }),
            CommandFunction("git merge", "マージ",
                            {CommandOption("なし", false, "", false),
                             CommandOption("-abort", false, "", false)}),
            CommandFunction("git remote", "リモート",
                            {CommandOption("-v", false, "", false),
                             CommandOption("set-url", true, "リモート名とURL", false)})};
}

QList<CommandFunction> getDockerCommands()
{
    return {
        CommandFunction("docker build", "ビルド",
                        {
                            CommandOption("なし", true, "", false),
                            CommandOption("-t", true, "名前とtag", false),
                            CommandOption("-f", true, "ファイル", false),
                        }),
        CommandFunction("docker images", "イメージ一覧を表示",
                        {
                            CommandOption("なし", true, "", false),
                            CommandOption("--all", false, "", false),
                            CommandOption("--quiet", false, "", false),
                        }),
        CommandFunction("docker run", "コンテナの作成、実行",
                        {
                            CommandOption("なし", true, "", false),
                        }),
        CommandFunction("docker image prune", "不要なイメージの削除",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker exec", "実行中のコンテナ内でコマンドを実行",
                        {
                            CommandOption("なし", true, "", false),
                        }),
        CommandFunction("docker pull", "リポジトリのダウンロード",
                        {
                            CommandOption("なし", true, "", false),
                        }),
        CommandFunction("docker push", "リポジトリにプッシュ",
                        {
                            CommandOption("なし", true, "", false),
                        }),
        CommandFunction("docker logs", "ログを表示",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker stop", "コンテナの停止",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker rm", "停止したコンテナの削除",
                        {
                            CommandOption("なし", true, "", false),
                        }),
        CommandFunction("docker rmi", "イメージの削除",
                        {
                            CommandOption("なし", true, "", false),
                        }),
        CommandFunction("docker network ls", "作成済みのネットワーク一覧を表示",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker volume ls", "作成済みのボリューム一覧を表示",
                        {
                            CommandOption("なし", false, "", false),
                        }),
    };
}

QList<CommandFunction> getDockerComposeCommands()
{
    return {
        CommandFunction("docker compose build", "サービスをビルド",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker compose exec", "実行中のコンテナ内でコマンドを実行",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker compose up", "コンテナを起動",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker compose down", "コンテナを停止しボリューム等を削除",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker compose restart", "コンテナを再起動",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker compose stop", "コンテナを停止",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker compose logs", "コンテナのログを表示",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker compose ps", "コンテナのステータスを表示",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker compose start", "停止中のサービスを起動",
                        {
                            CommandOption("なし", false, "", false),
                        }),
        CommandFunction("docker compose kill", "コンテナを強制終了",
                        {
                            CommandOption("なし", false, "", false),
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
    QMessageBox::critical(nullptr, "Error", "This value is invalid. Please check and try again.");
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
        if (option_list->isEnabled() && option.getName() != "なし") {
            command += " " + option.getName();
        }
        if (text_edit->isEnabled()) {
            if (option.isRequiredQuotes()) {
                command += " \"" + value1 + "\"";
            } else {
                command += " " + value1;
            }
        }
        text_browser->setText(command);
    }
}

void Command::copy()
{
    const QString text = text_browser->toPlainText();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void Command::adjustCommandBoxWidth()
{
    int maxWidth = 0;
    QFontMetrics const fontMetrics(functions_list->font());
    for (int i = 0; i < functions_list->count(); ++i) {
        int const width = fontMetrics.horizontalAdvance(functions_list->itemText(i));
        maxWidth = std::max(width, maxWidth);
    }
    functions_list->setMinimumWidth(maxWidth + 40);
}
