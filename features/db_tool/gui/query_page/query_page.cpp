#include "query_page.h"

#include "features/framework/gui/design_system.h"

#include <QEvent>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableView>
#include <QVBoxLayout>

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
    auto *verticalLayout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(verticalLayout);

    queryTextEdit = DevTools::Ui::createPlainTextEdit(this);
    DevTools::Ui::configureCodeEditor(queryTextEdit);
    verticalLayout->addWidget(queryTextEdit);

    auto *actionLayout = new QHBoxLayout();
    executeButton = new QPushButton(this);
    DevTools::Ui::configureCompactButton(executeButton);
    actionLayout->addWidget(executeButton);
    DevTools::Ui::configureActionBar(actionLayout, DevTools::Ui::ActionBarAlignment::Trailing);
    verticalLayout->addLayout(actionLayout);

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
        return;
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
