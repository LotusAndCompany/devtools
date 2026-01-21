#include "query_page.h"

#include "ui_query_page.h"

#include <QDebug>
#include <QEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableView>
#include <QTextEdit>
#include <QUuid>

QueryPage::QueryPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::QueryPage), model(new QSqlQueryModel(this)) // modelの初期化
{
    ui->setupUi(this);

    // DBの初期化
    db = QSqlDatabase::database();

    if (!db.open()) {
        QMessageBox::critical(this, "DB Error", "データベースに接続できませんでした");
        return;
    }

    // QTableViewにモデルをセット
    ui->queryResultView->setModel(model);

    // ボタンのクリックにスロットを接続
    connect(ui->executeButton, &QPushButton::clicked, this, &QueryPage::executeQuery);
}

void QueryPage::executeQuery()
{
    QString queryText = ui->queryTextEdit->toPlainText();
    QSqlQuery query(db);

    if (!query.exec(queryText)) {
        QMessageBox::warning(this, "実行エラー", query.lastError().text());
        return;
    }

    model->setQuery(std::move(query));

    if (model->lastError().isValid()) {
        QMessageBox::warning(this, "結果取得エラー", model->lastError().text());
    }

    QMessageBox::information(this, "成功", "クエリを正常に実行しました");
}

QueryPage::~QueryPage()
{
    delete ui;
}

void QueryPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    } else {
        QWidget::changeEvent(event);
    }
}
