#include "api_tool.h"
#include "ui_api_tool.h"

api_tool::api_tool(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::api_tool)
{
    ui->setupUi(this);
    qDebug() << "api_tool UI setup complete";

    qDebug() << "Children count:" << this->children().count();
    foreach(QObject *child, this->children()) {
        qDebug() << "Child widget:" << child;
    }
}

api_tool::~api_tool()
{
    qDebug() << "Destroying api_tool";
    delete ui;
}
