#include "contents_area.h"
#include "api_tool.h"
#include "ui_contents_area.h"

#include <QLabel>

ContentsArea::ContentsArea(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::ContentsArea)
{
    ui->setupUi(this);

    changeContent(Sidemenu::ID::HOME);
}

ContentsArea::~ContentsArea()
{
    delete ui;
}

void ContentsArea::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void ContentsArea::onSidemenuItemChanged(Sidemenu::ID id)
{
    changeContent(id);
}

void ContentsArea::changeContent(Sidemenu::ID id)
{
    if (currentContent) {
        ui->contentsAreaLayout->removeWidget(currentContent);
        // NOTE: 解放する
        delete currentContent;
        currentContent = nullptr;
    }

    QWidget *content = nullptr;

    switch (id) {
    case Sidemenu::ID::HOME:
        content = new QLabel("(´･ω･) ﾎｰﾑﾀﾞﾖｰ", this);
        static_cast<QLabel *>(content)->setAlignment(Qt::AlignCenter);
        break;
    case Sidemenu::ID::SAMPLE_0:
        content = new SampleGuiTool(Tool::ID::SAMPLE_0, "sample-0", this);
        break;
    case Sidemenu::ID::SAMPLE_1:
        // content = new SampleGuiTool(Tool::ID::SAMPLE_1, "sample-1", this);
        content = new api_tool(this);
        break;
    case Sidemenu::ID::SAMPLE_2:
        content = new SampleGuiTool(Tool::ID::SAMPLE_2, "sample-2", this);
        break;
    case Sidemenu::ID::SAMPLE_3:
        content = new SampleGuiTool(Tool::ID::SAMPLE_3, "sample-3", this);
        break;
    default:
        // NOTE: signal/slotでは例外を投げるべきではない
        content = new QLabel("Under development...", this);
        static_cast<QLabel *>(content)->setAlignment(Qt::AlignCenter);
        break;
    }

    currentContent = content;
    ui->contentsAreaLayout->addWidget(currentContent);
}
