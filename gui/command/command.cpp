#include "command.h"
#include "ui_command.h"
#include "command_function.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>

Command::Command(QWidget *parent)
    : QGroupBox(parent)
    , ui(new Ui::Command)
{
    ui->setupUi(this);

    Command::init();

    connect(ui->categoryList,
            &QComboBox::currentIndexChanged,
            this,
            &Command::selectedCategory);
    connect(ui->functionsList,
            &QComboBox::currentIndexChanged,
            this,
            &Command::selectedFunction);
    connect(ui->optionList,
            &QComboBox::currentIndexChanged,
            this,
            &Command::selectedOption);
    connect(ui->resetButton,
            &QPushButton::clicked,
            this,
            &Command::reset);
    connect(ui->generateButton,
            &QPushButton::clicked,
            this,
            &Command::generate);
    connect(ui->clearButton,
            &QPushButton::clicked,
            this,
            &Command::clear);
    connect(ui->copyButton,
            &QPushButton::clicked,
            this,
            &Command::copy);        
}

Command::~Command()
{
    delete ui;
}

void Command::init() {
    const QStringList categoryList {
        "カテゴリ一覧",
        "1: Gitコマンド",
        "2: Dockerコマンド",
        "3: DockerComposeコマンド"
    };

    // set categoryList items
    ui->categoryList->addItems(categoryList);

    // adjust commnadBox minmun width
    adjustCommandBoxWidth();

    // all label default hidden
    ui->label->setVisible(false);

    // all textBox default hidden
    ui->textEdit->setVisible(false);

    // commandList default hidden
    ui->functionsLabel->setVisible(false);
    ui->functionsList->setVisible(false);
    // optionList default hidden
    ui->optionLabel->setVisible(false);
    ui->optionList->setVisible(false);
}

QList<CommandFunction> getGitCommands() {
    return {
        CommandFunction("git add", "変更を追加", {
            CommandOption("なし", true, "Path", false),
        }),
        CommandFunction("git commit", "コミットを作成", {
            CommandOption("なし", false, "", false),
            CommandOption("-m", true, "メッセージ", true),
            CommandOption("-a", false, "", false)
        }),
        CommandFunction("git log", "コミット履歴を確認", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("git status", "ブランチの状態確認", {
            CommandOption("なし", false, "", false),
        }),
        /*
        CommandFunction("git reset", "直前のコミットを取り消し", {
            CommandOption("-soft", true, ""),
            CommandOption("-hard", true, ""),
        }),
        CommandFunction("git config", "利用中のアカウントの情報確認", {
            CommandOption("なし", false, "", false),
        }),
        */
        CommandFunction("git diff", "差分を確認", {
            CommandOption("なし", true, "比較対象", false),
        }),
        CommandFunction("git merge", "マージ", {
            CommandOption("なし", false, "", false),
            CommandOption("-abort", false, "", false)
        }),
        CommandFunction("git remote", "リモート", {
            CommandOption("-v", false, "", false),
            CommandOption("set-url", true, "リモート名とURL", false)
        })
    };
}

QList<CommandFunction> getDockerCommands() {
    return {
        CommandFunction("docker build", "ビルド", {
            CommandOption("なし", true, "", false),
            CommandOption("-t", true, "名前とtag", false),
            CommandOption("-f", true, "ファイル", false),
        }),
        CommandFunction("docker images", "イメージ一覧を表示", {
            CommandOption("なし", true, "", false),
            CommandOption("--all", false, "", false),
            CommandOption("--quiet", false, "", false),
        }),
        CommandFunction("docker run", "コンテナの作成、実行", {
            CommandOption("なし", true, "", false),
        }),
        CommandFunction("docker image prune", "不要なイメージの削除", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker exec", "実行中のコンテナ内でコマンドを実行", {
            CommandOption("なし", true, "", false),
        }),
        CommandFunction("docker pull", "リポジトリのダウンロード", {
            CommandOption("なし", true, "", false),
        }),
        CommandFunction("docker push", "リポジトリにプッシュ", {
            CommandOption("なし", true, "", false),
        }),
        CommandFunction("docker logs", "ログを表示", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker stop", "コンテナの停止", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker rm", "停止したコンテナの削除", {
            CommandOption("なし", true, "", false),
        }),
        CommandFunction("docker rmi", "イメージの削除", {
            CommandOption("なし", true, "", false),
        }),
        CommandFunction("docker network ls", "作成済みのネットワーク一覧を表示", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker volume ls", "作成済みのボリューム一覧を表示", {
            CommandOption("なし", false, "", false),
        }),
    };
}

QList<CommandFunction> getDockerComposeCommands() {
    return {
        CommandFunction("docker compose build", "サービスをビルド", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker compose exec", "実行中のコンテナ内でコマンドを実行", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker compose up", "コンテナを起動", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker compose down", "コンテナを停止しボリューム等を削除", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker compose restart", "コンテナを再起動", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker compose stop", "コンテナを停止", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker compose logs", "コンテナのログを表示", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker compose ps", "コンテナのステータスを表示", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker compose start", "停止中のサービスを起動", {
            CommandOption("なし", false, "", false),
        }),
        CommandFunction("docker compose kill", "コンテナを強制終了", {
            CommandOption("なし", false, "", false),
        }),
    };
}

void Command::selectedCategory()
{
    const int selectedIndex = ui->categoryList->currentIndex();

    ui->functionsList->clear();

    QList<CommandFunction> commandList;
    QStringList commandNames;

    // set list items
    switch (selectedIndex) {
    case 1:        
        commandList = getGitCommands();
        for (const CommandFunction& cmd : commandList) {
            commandNames.append(cmd.getName() + " " + cmd.getDescription());
        }
        ui->functionsList->addItems(commandNames);
        // display commandList
        ui->functionsLabel->setVisible(true);
        ui->functionsList->setVisible(true);
        ui->functionsList->setEnabled(true);
        break;
    case 2:
        commandList = getDockerCommands();
        for (const CommandFunction& cmd : commandList) {
            commandNames.append(cmd.getName() + " " + cmd.getDescription());
        }
        ui->functionsList->addItems(commandNames);
        // display commandList
        ui->functionsLabel->setVisible(true);
        ui->functionsList->setVisible(true);
        ui->functionsList->setEnabled(true);
        break;
    case 3:
        commandList = getDockerComposeCommands();
        for (const CommandFunction& cmd : commandList) {
            commandNames.append(cmd.getName() + " " + cmd.getDescription());
        }
        ui->functionsList->addItems(commandNames);
        // display commandList
        ui->functionsLabel->setVisible(true);
        ui->functionsList->setVisible(true);
        ui->functionsList->setEnabled(true);
        break;
    default:
        reset();
        break;
    }
}

void Command::selectedFunction()
{
    // all textBox clear
    ui->textEdit->clear();
    // optionList clear
    ui->optionList->clear();
    ui->optionList->setVisible(false);
    ui->optionList->setEnabled(false);

    const int selectedCategoryIndex = ui->categoryList->currentIndex();
    const int selectedFunctionsIndex = ui->functionsList->currentIndex();

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

    if (selectedFunctionsIndex < 0 || selectedFunctionsIndex >= commandList.size()) return;
    QList<CommandOption> optionList = commandList[selectedFunctionsIndex].getOptions();

    if (optionList.length()) {
        ui->label->setVisible(false);
        ui->textEdit->setVisible(false);
        // display optionsList
        QStringList optionNames;

        for (const CommandOption& opt : optionList) {
            optionNames.append(opt.getName());
        }
        ui->optionList->addItems(optionNames);
        ui->optionLabel->setVisible(true);
        ui->optionList->setVisible(true);
        ui->optionList->setEnabled(true);
    }
}

void Command::selectedOption()
{
    const int selectedCategoryIndex = ui->categoryList->currentIndex();
    const int selectedFunctionsIndex = ui->functionsList->currentIndex();
    const int selectedOptionIndex = ui->optionList->currentIndex();

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
    if (selectedFunctionsIndex < 0 || selectedFunctionsIndex >= commandList.size()) return;

    QList<CommandOption> optionList = commandList[selectedFunctionsIndex].getOptions();
    if (selectedOptionIndex < 0 || selectedOptionIndex >= optionList.size()) return;

    CommandOption option = optionList[selectedOptionIndex];

    if (option.isRequired()) {
        ui->label->setText(option.getTitle());
        ui->label->setVisible(true);
        ui->textEdit->setVisible(true);
    } else {
        ui->label->setVisible(false);
        ui->textEdit->setVisible(false);
    }

}

void Command::reset()
{
    // reset list current index
    ui->categoryList->setCurrentIndex(0);

    ui->functionsList->clear();
    ui->optionList->clear();

    // all lineEdit clear
    ui->textEdit->clear();

    // all label default hidden
    ui->label->setVisible(false);

    // all lineEdit default hidden
    ui->textEdit->setVisible(false);

    // commandList default hidden
    ui->functionsLabel->setVisible(false);
    ui->functionsList->setVisible(false);
    ui->functionsList->setEnabled(false);

    // optionList default hidden
    ui->optionLabel->setVisible(false);
    ui->optionList->setVisible(false);
    ui->optionList->setEnabled(false);
}

void Command::clear()
{
    ui->textBrowser->clear();
}


/**
 * @brief Make sure the string does not contain "'`
 * @param Qstring ui->textEdit->text()
 * @return bool
 */
bool containsNoQuotes(const QString &str) {
    QString quotes = "\"'`";

    for (const QChar &quote : quotes) {
        if (str.contains(quote)) {
            return false;
        }
    }
    return true;
}

void showErrorAlert() {
    QMessageBox::critical(nullptr, "Error", "This value is invalid. Please check and try again.");
}

void Command::generate()
{
    const int selectedCategoryIndex = ui->categoryList->currentIndex();
    const int selectedFunctionsIndex = ui->functionsList->currentIndex();
    const int selectedOptionIndex = ui->optionList->currentIndex();

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
    if (selectedFunctionsIndex < 0 || selectedFunctionsIndex >= commandList.size()) return;

    QList<CommandOption> optionList = commandList[selectedFunctionsIndex].getOptions();
    if (selectedOptionIndex < 0 || selectedOptionIndex >= optionList.size()) return;

    CommandOption option = optionList[selectedOptionIndex];

    const QString value1 = ui->textEdit->text();

    // simple validation
    if (!containsNoQuotes(value1)) {
        showErrorAlert();
    } else {
        QString command = commandList[selectedFunctionsIndex].getName();
        if (ui->optionList->isEnabled() && option.getName() != "なし") {
            command += " " + option.getName();
        }
        if (ui->textEdit->isEnabled()) {
            if (option.isRequiredQuotes()) {
                command += " \"" + value1 + "\"";
            } else {
                command += " " + value1;
            }
        }
        ui->textBrowser->setText(command);
    }

}

void Command::copy()
{
    const QString text = ui->textBrowser->toPlainText();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void Command::adjustCommandBoxWidth()
{
    int maxWidth = 0;
    QFontMetrics fontMetrics(ui->functionsList->font());
    for (int i = 0; i < ui->functionsList->count(); ++i) {
        int width = fontMetrics.horizontalAdvance(ui->functionsList->itemText(i));
        if (width > maxWidth) {
            maxWidth = width;
        }
    }
    ui->functionsList->setMinimumWidth(maxWidth + 40);
}
