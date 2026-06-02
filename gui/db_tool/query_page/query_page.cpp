#include "query_page.h"

#include <QEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QSizePolicy>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {
constexpr int DEFAULT_WIDTH = 400;
constexpr int DEFAULT_HEIGHT = 300;
} // namespace

QueryPage::QueryPage(QWidget *parent) : QWidget(parent), model(new QSqlQueryModel(this))
{
    buildUi();

    db = QSqlDatabase::database();

    if (!db.open()) {
        QMessageBox::critical(this, tr("DB Error"), tr("Could not connect to the database."));
        return;
    }

    queryResultView->setModel(model);

    connect(executeButton, &QPushButton::clicked, this, &QueryPage::executeQuery);
}

void QueryPage::buildUi()
{
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    auto *verticalLayout = new QVBoxLayout(this);

    queryTextEdit = new QTextEdit(this);
    verticalLayout->addWidget(queryTextEdit);

    executeButton = new QPushButton(this);
    executeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addWidget(executeButton);

    queryResultView = new QTableView(this);
    verticalLayout->addWidget(queryResultView);

    retranslateUi();
}

void QueryPage::retranslateUi()
{
    setWindowTitle(tr("Form"));
    executeButton->setText(tr("Execute"));
}

void QueryPage::executeQuery()
{
    QString const queryText = queryTextEdit->toPlainText();
    QSqlQuery query(db);

    if (!query.exec(queryText)) {
        QMessageBox::warning(this, tr("Execution Error"), query.lastError().text());
        return;
    }

    model->setQuery(std::move(query));

    if (model->lastError().isValid()) {
        QMessageBox::warning(this, tr("Result Fetch Error"), model->lastError().text());
    }

    QMessageBox::information(this, tr("Success"), tr("Query executed successfully."));
}

void QueryPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else {
        QWidget::changeEvent(event);
    }
}
