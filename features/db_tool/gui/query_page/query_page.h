#ifndef QUERY_PAGE_H
#define QUERY_PAGE_H

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QWidget>

class QPushButton;
class QPlainTextEdit;
class QTableView;

class QueryPage : public QWidget
{
    Q_OBJECT

public:
    explicit QueryPage(QWidget *parent = nullptr);
    ~QueryPage() override = default;
    QueryPage(const QueryPage &) = delete;
    QueryPage &operator=(const QueryPage &) = delete;
    QueryPage(QueryPage &&) = delete;
    QueryPage &operator=(QueryPage &&) = delete;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void executeQuery();

private:
    void buildUi();
    void retranslateUi();

    QPlainTextEdit *queryTextEdit{nullptr};
    QPushButton *executeButton{nullptr};
    QTableView *queryResultView{nullptr};

    QSqlDatabase db;
    QSqlQueryModel *model{nullptr};
};

#endif // QUERY_PAGE_H
