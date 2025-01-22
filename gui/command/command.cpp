#include "command.h"
#include "ui_command.h"
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
        "2: Dockerコマンド"
    };

    // set categoryList items
    ui->categoryList->addItems(categoryList);

    // adjust commnadBox minmun width
    adjustCommandBoxWidth();

    // all label default hidden
    ui->label->setVisible(false);

    // all textBox default hidden
    ui->textEdit->setVisible(false);

    // functionsList default hidden
    ui->functionsList->setVisible(false);
}

const QStringList gitFunctionsList {
    "機能一覧",
    "1: 変更を追加",
    "2: コミット",
    "3: コミット & コメント",
    "4: ブランチの状態確認",
    "5: 直前のコミットを取り消し",
    "6: 直前のコミットと変更内容を取り消し",
    "7: マージ",
    "8: マージする際、コンフリクトの編集を破棄",
    "9: リモートリポジトリの一覧確認",
    "10: リモートリポジトリ オリジンのurlを変更",
    "11: コミット履歴を確認",
    // git diff
    // git config 利用中のアカウントの情報確認
};

const QStringList dockerFunctionsList {
    "機能一覧",
    "1:  ビルド", // docker build
    "2:  イメージ一覧を表示", // docker images
    "3:  コンテナの作成、実行", // docker run
    "4:  実行中のコンテナを表示", // docker ps
    "5:  不要なイメージの削除", // docker image prune
    "6:  実行中のコンテナ内でコマンドを実行", // docker exec
    "7:  リポジトリのダウンロード", // docker pull
    "8:  リポジトリにプッシュ", // docker push
    "9:  ログを表示", // docker logs
    "10: コンテナの停止", // docker stop
    "11: 停止したコンテナの削除", // docker rm
    "12: イメージの削除", // docker rmi
    "13: 作成済みのネットワーク一覧を表示", // docker network ls
    "14: 作成済みのボリューム一覧を表示", // docker volume ls
    "",
    "----Docker Compose----",
    "15: コンテナを起動", // docker compose up
    "16: コンテナを停止しボリューム等を削除", // docker compose down
    "17: コンテナを再起動", // docker compose restart
    "18: サービスをビルド", // docker compose build
    "19: コンテナのログを表示", // docker compose logs
    "20: コンテナのステータスを表示", // docker compose ps
    "21: コンテナを停止", // docker compose stop
    "22: 停止中のサービスを起動", // docker compose start
    "23: 実行中のコンテナ内でコマンドを実行", // docker compose exec
    "24: コンテナを強制終了", // docker compose kill

};

void Command::selectedCategory()
{
    const int selectedIndex = ui->categoryList->currentIndex();

    ui->functionsList->clear();

    // set list items
    switch (selectedIndex) {
    case 1:
        ui->functionsList->addItems(gitFunctionsList);
        // display functionsList
        ui->functionsList->setVisible(true);
        ui->functionsList->setEnabled(true);
        break;
    case 2:
        ui->functionsList->addItems(dockerFunctionsList);
        // display functionsList
        ui->functionsList->setVisible(true);
        ui->functionsList->setEnabled(true);
        break;
    }
}

void Command::selectedFunction()
{
    // all textBox clear
    ui->textEdit->clear();

    const int selectedCategoryIndex = ui->categoryList->currentIndex();
    switch (selectedCategoryIndex) {
    case 1:
        Command::selectedGitFunction();
        break;
    case 2:
        Command::selectedDockerFunction();
        break;
    default:
        break;
    }
}

void Command::selectedGitFunction() {
    const int selectedIndex = ui->functionsList->currentIndex();
    switch (selectedIndex) {
    case 1:
        ui->label->setText("Path");
        ui->label->setVisible(true);
        ui->textEdit->setVisible(true);
        break;
    case 3:
        ui->label->setText("Comment");
        ui->label->setVisible(true);
        ui->textEdit->setVisible(true);
        break;
    case 7:
        ui->label->setText("Branch");
        ui->label->setVisible(true);
        ui->textEdit->setVisible(true);
        break;
    case 10:
        ui->label->setText("New URL");
        ui->label->setVisible(true);
        ui->textEdit->setVisible(true);
        break;
    case 2:
    case 4:
    case 5:
    case 6:
    case 8:
    case 9:
    case 11:
    default:
        ui->label->setVisible(false);
        ui->textEdit->setVisible(false);
        break;
    }
}

void Command::selectedDockerFunction() {
    const int selectedIndex = ui->functionsList->currentIndex();
    switch (selectedIndex) {
    // set Docker command input parmater title to label.
    case 1:  // docker build
    case 3:  // docker run
    case 4:  // docker ps
    case 5:  // docker image prune
    case 17: // docker compose up
    case 18: // docker compose down
    case 21: // docker compose ps
        ui->label->setText("option");
        ui->label->setVisible(true);
        ui->textEdit->setVisible(true);
        break;
    case 2:  // docker images
    case 6:  // docker exec
    case 7:  // docker pull
    case 8:  // docker push
    case 9:  // docker logs
    case 10: // docker stop
    case 11: // docker rm
    case 12: // docker rmi
    case 13: // docker network ls
    case 14: // docker volume ls
    case 19: // docker compose build
    case 20: // docker compose logs
    case 22: // docker compose stop
    case 23: // docker compose start
    case 24: // docker compose exec
    case 25: // docker compose kill
    default:
        ui->label->setVisible(false);
        ui->textEdit->setVisible(false);
        break;
    }
}

void Command::reset()
{
    ui->functionsList->setCurrentIndex(0);

    // all lineEdit clear
    ui->textEdit->clear();

    // all label default hidden
    ui->label->setVisible(false);

    // all lineEdit default hidden
    ui->textEdit->setVisible(false);
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
    // Todo: add Docker command generation
    if (ui->categoryList->currentIndex() != 1) return;
    const QString gitAdd = "git add ";
    const QString gitCommit = "git commit";
    const QString gitCommitComment = "git commit -m ";
    const QString gitStatus = "git status";
    const QString gitResetSoft = "git reset --soft HEAD^";
    const QString gitResetHard = "git reset --hard HEAD^";
    const QString gitMerge = "git merge ";
    const QString gitMergeAbort = "git merge --abort";
    const QString gitRemoteList = "git remote -v";
    const QString gitRemoteOriginSetURL = "git remote set-url origin ";
    const QString gitLog = "git log";

    const QString dockerBuild           = "docker build";           // 1
    const QString dockerImages          = "docker images";          // 2
    const QString dockerRun             = "docker run";             // 3
    const QString dockerPs              = "docker ps";              // 4
    const QString dockerImagePure       = "docker image pure";      // 5
    const QString dockerExec            = "docker exec";            // 6
    const QString dockerPull            = "docker pull";            // 7
    const QString dockerPush            = "docker push";            // 8
    const QString dockerLogs            = "docker logs";            // 9
    const QString dockerStop            = "docker stop";            // 10
    const QString dockerRm              = "docker rm";              // 11
    const QString dockerRmi             = "docker rmi";             // 12
    const QString dockerNetworkLs       = "docker network ls";      // 13
    const QString dockerVolumeLs        = "docker volume ls";       // 14
    const QString dockerComposeUp       = "docker compose up";      // 17
    const QString dockerComposeDown     = "docker compose down";    // 18
    const QString dockerComposeBuild    = "docker compose build";   // 19
    const QString dockerComposeLogs     = "docker compose logs";    // 20
    const QString dockerComposePs       = "docker compose ps";      // 21
    const QString dockerComposeStop     = "docker compose stop";    // 22
    const QString dockerComposeStart    = "docker compose start";   // 23
    const QString dockerComposeExec     = "docker compose exec";    // 24
    const QString dockerComposeKill     = "docker compose kill";    // 25

    const int selectedIndex = ui->functionsList->currentIndex();
    const QString value1 = ui->textEdit->text();

    // simple validation
    if (!containsNoQuotes(value1)) {
        showErrorAlert();
    } else {
        switch (selectedIndex) {
        case 1:
            ui->textBrowser->setText(gitAdd + value1);
            break;
        case 2:
            ui->textBrowser->setText(gitCommit);
            break;
        case 3:
            ui->textBrowser->setText(gitCommitComment + "'" + value1 + "'");
            break;
        case 4:
            ui->textBrowser->setText(gitStatus);
            break;
        case 5:
            ui->textBrowser->setText(gitResetSoft);
            break;
        case 6:
            ui->textBrowser->setText(gitResetHard);
            break;
        case 7:
            ui->textBrowser->setText(gitMerge + value1);
            break;
        case 8:
            ui->textBrowser->setText(gitMergeAbort);
            break;
        case 9:
            ui->textBrowser->setText(gitRemoteList);
            break;
        case 10:
            ui->textBrowser->setText(gitRemoteOriginSetURL + value1);
            break;
        case 11:
            ui->textBrowser->setText(gitLog);
            break;
        default:
            break;
        }
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
