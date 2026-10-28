#include "features/db_tool/core/query_service.h"

#include <QSqlQueryModel>
#include <QtTest>

#include <utility>

class TestQueryService : public QObject
{
    Q_OBJECT

private slots:
    void executesAndFetches();
};

void TestQueryService::executesAndFetches()
{
    const QString name = QStringLiteral("query-service-test");
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", name);
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());
        QVERIFY(QueryService::execute(db, "CREATE TABLE items (id INTEGER, value TEXT)").success);
        QVERIFY(QueryService::execute(db, "INSERT INTO items VALUES (1, '日本語'), (2, 'two')")
                    .success);
        QueryExecution result =
            QueryService::execute(db, "SELECT id, value FROM items ORDER BY id");
        QVERIFY(result.success);
        QVERIFY(!result.error.isValid());
        QSqlQueryModel model;
        model.setQuery(std::move(result.query));
        QVERIFY(!model.lastError().isValid());
        QCOMPARE(model.columnCount(), 2);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.data(model.index(0, 1)).toString(), QStringLiteral("日本語"));
        QCOMPARE(model.data(model.index(1, 0)).toInt(), 2);
        QueryExecution large = QueryService::execute(
            db,
            "WITH RECURSIVE seq(n) AS (VALUES(1) UNION ALL SELECT n+1 FROM seq WHERE n<600) "
            "SELECT n FROM seq");
        QVERIFY(large.success);
        QSqlQueryModel large_model;
        large_model.setQuery(std::move(large.query));
        while (large_model.canFetchMore()) {
            large_model.fetchMore();
        }
        QCOMPARE(large_model.rowCount(), 600);
        QCOMPARE(large_model.data(large_model.index(599, 0)).toInt(), 600);
        const QueryExecution error = QueryService::execute(db, "SELECT * FROM missing_table");
        QVERIFY(!error.success);
        QVERIFY(error.error.isValid());
        QVERIFY(!error.error.text().isEmpty());
        db.close();
        const QueryExecution closed = QueryService::execute(db, "SELECT 1");
        QVERIFY(!closed.success);
    }
    QSqlDatabase::removeDatabase(name);
}

QTEST_GUILESS_MAIN(TestQueryService)
#include "test_query_service.moc"
