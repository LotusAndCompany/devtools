#include "command.h"
#include "ui_command.h"

Command::Command(QWidget *parent)
    : QGroupBox(parent)
    , ui(new Ui::Command)
{
    ui->setupUi(this);
}

Command::~Command()
{
    delete ui;
}
