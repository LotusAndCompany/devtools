#include "command.h"
#include "ui_command.h"

Command::Command(QWidget *parent)
    : QGroupBox(parent)
    , ui(new Ui::Command)
{
    ui->setupUi(this);

    Command::init();
}

Command::~Command()
{
    delete ui;
}

void Command::init() {
    const QStringList CommandList {
        "command list",
        "git add",
        "git commit",
        "git log",
        "git status",
        "git checkout",
        "git diff",
        "git reset",
        "git branch",
        "git merge",
        "git clone",
        "git pull",
        "git push",
    };

    ui->commandBox->addItems(CommandList);

    // hidden unused object
    ui->categoryBox->setVisible(false);
    ui->optionsBox->setVisible(false);

    // all textBox default hidden
    ui->textEdit->setVisible(false);
    ui->textEdit_2->setVisible(false);
    ui->textEdit_3->setVisible(false);
}

void Command::reset()
{
    ui->commandBox->clear();
    ui->optionsBox->clear();
    ui->textEdit->clear();
    ui->textEdit_2->clear();
    ui->textEdit_3->clear();
}
