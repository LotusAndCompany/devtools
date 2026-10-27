#include "features/db_tool/gui/query_page/query_page.h"

#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSqlQuery>
#include <QTableView>
#include <QTimer>
#include <QtTest>

class TestQueryPage : public QObject
{
    Q_OBJECT

private slots:
    void executeAndRetainResultsOnError();
};

void TestQueryPage::executeAndRetainResultsOnError()
{
    const QString name = QStringLiteral("query-page-test");
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", name);
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());
        QSqlQuery setup(db);
        QVERIFY(setup.exec("CREATE TABLE items (value TEXT)"));
        QVERIFY(setup.exec("INSERT INTO items VALUES ('one'), ('two')"));
        QueryPage page(db);
        auto *editor = page.findChild<QPlainTextEdit *>();
        auto *button = page.findChild<QPushButton *>();
        auto *table = page.findChild<QTableView *>();
        QVERIFY(editor != nullptr);
        QVERIFY(button != nullptr);
        QVERIFY(table != nullptr);
        QString dialog_text;
        QTimer timer;
        connect(&timer, &QTimer::timeout, &page, [&] {
            for (auto *widget : QApplication::topLevelWidgets()) {
                if (auto *message = qobject_cast<QMessageBox *>(widget)) {
                    dialog_text = message->text();
                    message->accept();
                }
            }
        });
        timer.start(10);
        editor->setPlainText("SELECT value FROM items ORDER BY value");
        button->click();
        QCOMPARE(dialog_text, QString("Query executed successfully."));
        QCOMPARE(table->model()->rowCount(), 2);
        QCOMPARE(table->model()->data(table->model()->index(0, 0)).toString(), QString("one"));
        editor->setPlainText("SELECT * FROM missing_table");
        button->click();
        QVERIFY(dialog_text.contains("missing_table"));
        QCOMPARE(table->model()->rowCount(), 2);
        QCOMPARE(table->model()->data(table->model()->index(0, 0)).toString(), QString("one"));
    }
    QSqlDatabase::removeDatabase(name);
}

QTEST_MAIN(TestQueryPage)
#include "test_query_page.moc"
