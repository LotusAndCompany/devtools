#ifndef QUERY_PAGE_H
#define QUERY_PAGE_H

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QWidget>

namespace Ui {
class QueryPage;
}

class QueryPage : public QWidget
{
    Q_OBJECT

public:
    explicit QueryPage(QWidget *parent = nullptr);
    ~QueryPage() override;
    QueryPage(const QueryPage &) = delete;
    QueryPage &operator=(const QueryPage &) = delete;
    QueryPage(QueryPage &&) = delete;
    QueryPage &operator=(QueryPage &&) = delete;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void executeQuery();

private:
    Ui::QueryPage *ui;
    QSqlDatabase db;
    QSqlQueryModel *model;
};

#endif // QUERY_PAGE_H
