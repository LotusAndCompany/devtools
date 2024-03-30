#include "contents_area.h"
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
        // TODO: ここで削除してはいけない場合の処理を追加する
        delete currentContent;
    }

    QLabel *const content = new QLabel(this);
    content->setAlignment(Qt::AlignCenter);

    switch (id) {
    case Sidemenu::ID::HOME:
        content->setText("(´･ω･) ﾎｰﾑﾀﾞﾖｰ");
        break;
    /*
    case Sidemenu::ItemID::SAMPLE_0:
        content->setText("サンプル0");
        break;
    case Sidemenu::ItemID::SAMPLE_1:
        content->setText("サンプル1");
        break;
    case Sidemenu::ItemID::SAMPLE_2:
        content->setText("サンプル2");
        break;
    case Sidemenu::ItemID::SAMPLE_3:
        content->setText("サンプル3");
        break;
    */
    default:
        content->setText("Other");
        break;
    }

    currentContent = content;
    ui->contentsAreaLayout->addWidget(currentContent);
}
