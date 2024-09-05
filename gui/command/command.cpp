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
    const QStringList functionsList {
        "Functions list",
        "1: 変更を追加",
        "2: コミット",
        "3: コミット & コメント",
        "4: ブランチの状態確認",
        "5: 直前のコミットを取り消し",
        "6: 直前のコミットと変更内容を取り消し",
        "7: マージ",
        "8: マージする際、コンフリクトの編集を破棄",
    };

    ui->functionsList->addItems(functionsList);
    // adjust commnadBox minmun width
    adjustCommandBoxWidth();

    // all label default hidden
    ui->label->setVisible(false);

    // all textBox default hidden
    ui->textEdit->setVisible(false);
}

void Command::selectedFunction()
{
    // all textBox clear
    ui->textEdit->clear();

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
    case 2:
    case 4:
    case 5:
    case 6:
    case 8:
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
    const QString gitAdd = "git add ";
    const QString gitCommit = "git commit";
    const QString gitCommitComment = "git commit -m ";
    const QString gitStatus = "git status";
    const QString gitResetSoft = "git reset --soft HEAD^";
    const QString gitResetHard = "git reset --hard HEAD^";
    const QString gitMerge = "git merge ";
    const QString gitMergeAbort = "git merge --abort";
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
