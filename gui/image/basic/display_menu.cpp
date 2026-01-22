#include "display_menu.h"

#include <QDropEvent>
#include <QGraphicsOpacityEffect>
#include <QResizeEvent>
#include <QWheelEvent>
#include <cmath>

BasicDisplayMenu::BasicDisplayMenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BasicImageView)
{
    ui->setupUi(this);

    // メインレイアウト
    QVBoxLayout *layout = new QVBoxLayout(this);

    // 「機能一覧」メニュー作成
    QMenu *functionsMenu = new QMenu("機能一覧", this);
    menuBar->addMenu(functionsMenu);

    // チェック可能なアクションを追加
    QAction *feature1 = new QAction("機能A", this);
    feature1->setCheckable(true);
    QAction *feature2 = new QAction("機能B", this);
    feature2->setCheckable(true);
    QAction *feature3 = new QAction("機能C", this);
    feature3->setCheckable(true);

    // メニューに追加
    functionsMenu->addAction(feature1);
    functionsMenu->addAction(feature2);
    functionsMenu->addAction(feature3);

    // チェック状態変更のシグナル接続
    connect(feature1, &QAction::toggled, this, [](bool checked){
        qDebug() << "機能A:" << (checked ? "ON" : "OFF");
    });
    connect(feature2, &QAction::toggled, this, [](bool checked){
        qDebug() << "機能B:" << (checked ? "ON" : "OFF");
    });
    connect(feature3, &QAction::toggled, this, [](bool checked){
        qDebug() << "機能C:" << (checked ? "ON" : "OFF");
    });

}

BasicDisplayMenu::~BasicDisplayMenu()
{
    delete ui;
}
