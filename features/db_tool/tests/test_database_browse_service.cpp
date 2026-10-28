#include "features/db_tool/core/database_browse_service.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QUuid>
#include <QtTest>

namespace {
class SQLiteFixture
{
public:
    SQLiteFixture()
        : name(QUuid::createUuid().toString(QUuid::WithoutBraces))
        , database(QSqlDatabase::addDatabase("QSQLITE", name))
    {
        database.setDatabaseName(":memory:");
    }

    ~SQLiteFixture()
    {
        database.close();
        database = QSqlDatabase();
        QSqlDatabase::removeDatabase(name);
    }

    QString name;
    QSqlDatabase database;
};
} // namespace

class TestDatabaseBrowseService : public QObject
{
    Q_OBJECT

private slots:
    void validatesAndListsTables();
    void createsTableModel();
};

void TestDatabaseBrowseService::validatesAndListsTables()
{
    SQLiteFixture fixture;
    QVERIFY(fixture.database.open());
    QVERIFY(!DatabaseBrowseService::validateSQLiteDatabase(fixture.database).isValid());

    QSqlQuery query(fixture.database);
    QVERIFY(query.exec("CREATE TABLE items(id INTEGER PRIMARY KEY, name TEXT)"));
    QVERIFY(query.exec("INSERT INTO items(name) VALUES('sample')"));
    QVERIFY(DatabaseBrowseService::tableNames(fixture.database).contains("items"));
}

void TestDatabaseBrowseService::createsTableModel()
{
    SQLiteFixture fixture;
    QVERIFY(fixture.database.open());
    QSqlQuery query(fixture.database);
    QVERIFY(query.exec("CREATE TABLE items(id INTEGER PRIMARY KEY, name TEXT)"));
    QVERIFY(query.exec("INSERT INTO items(name) VALUES('sample')"));

    QObject modelOwner;
    QSqlTableModel *model =
        DatabaseBrowseService::tableModel(fixture.database, "items", &modelOwner);
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->record(0).value("name").toString(), QString("sample"));
}

QTEST_GUILESS_MAIN(TestDatabaseBrowseService)
#include "test_database_browse_service.moc"
