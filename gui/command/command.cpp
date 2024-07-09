#include "command.h"
#include "ui_command.h"
#include <QClipboard>
#include <QApplication>

Command::Command(QWidget *parent)
    : QGroupBox(parent)
    , ui(new Ui::Command)
{
    ui->setupUi(this);

    Command::init();

    connect(ui->commandBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
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
    const QStringList CommandList {
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

    ui->commandBox->addItems(CommandList);
    // adjust commnadBox minmun width
    adjustCommandBoxWidth();

    // hidden unused object
    //ui->categoryBox->setVisible(false);
    //ui->optionsBox->setVisible(false);

    // all label default hidden
    ui->label->setVisible(false);
    ui->label_2->setVisible(false);
    ui->label_3->setVisible(false);

    // all textBox default hidden
    ui->textEdit->setVisible(false);
    ui->textEdit_2->setVisible(false);
    ui->textEdit_3->setVisible(false);
}

void Command::selectedFunction()
{
    // all textBox clear
    ui->textEdit->clear();
    ui->textEdit_2->clear();
    ui->textEdit_3->clear();

    const int selectedIndex = ui->commandBox->currentIndex();
    switch (selectedIndex) {
    case 1:
        ui->label->setText("Path");
        ui->label->setVisible(true);
        ui->textEdit->setVisible(true);
        ui->textEdit_2->setVisible(false);
        ui->textEdit_3->setVisible(false);
        break;
    case 3:
        ui->label->setText("Comment");
        ui->label->setVisible(true);
        ui->textEdit->setVisible(true);
        ui->textEdit_2->setVisible(false);
        ui->textEdit_3->setVisible(false);
        break;
    case 7:
        ui->label->setText("Branch");
        ui->label->setVisible(true);
        ui->textEdit->setVisible(true);
        ui->textEdit_2->setVisible(false);
        ui->textEdit_3->setVisible(false);
        break;
    case 2:
    case 4:
    case 5:
    case 6:
    case 8:
    default:
        ui->label->setVisible(false);
        ui->label_2->setVisible(false);
        ui->label_3->setVisible(false);
        ui->textEdit->setVisible(false);
        ui->textEdit_2->setVisible(false);
        ui->textEdit_3->setVisible(false);
        break;
    }
}

void Command::reset()
{
    ui->commandBox->setCurrentIndex(0);

    // all textBox clear
    ui->textEdit->clear();
    ui->textEdit_2->clear();
    ui->textEdit_3->clear();

    // hidden unused object
    //ui->categoryBox->setVisible(false);
    //ui->optionsBox->setVisible(false);

    // all label default hidden
    ui->label->setVisible(false);
    ui->label_2->setVisible(false);
    ui->label_3->setVisible(false);

    // all textBox default hidden
    ui->textEdit->setVisible(false);
    ui->textEdit_2->setVisible(false);
    ui->textEdit_3->setVisible(false);
}

void Command::clear()
{
    ui->textBrowser->clear();
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
    const int selectedIndex = ui->commandBox->currentIndex();
    const QString value1 = ui->textEdit->toPlainText();
    //const QString value2 = ui->textEdit_2->toPlainText();
    //const QString value3 = ui->textEdit_3->toPlainText();

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

void Command::copy()
{
    const QString text = ui->textBrowser->toPlainText();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void Command::adjustCommandBoxWidth()
{
    int maxWidth = 0;
    QFontMetrics fontMetrics(ui->commandBox->font());
    for (int i = 0; i < ui->commandBox->count(); ++i) {
        int width = fontMetrics.horizontalAdvance(ui->commandBox->itemText(i));
        if (width > maxWidth) {
            maxWidth = width;
        }
    }
    ui->commandBox->setMinimumWidth(maxWidth + 40);
}
