#include "features/db_tool/core/database_connection.h"
#include "features/db_tool/core/sqlite_file_access.h"
#include "features/db_tool/core/sqlite_paths.h"

#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>

class TestDatabaseConnection : public QObject
{
    Q_OBJECT

private slots:
    void openAndReopen();
    void metadata();
    void pathsAndAccess();
};

void TestDatabaseConnection::openAndReopen()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString name = QStringLiteral("database-connection-test");
    {
        const QString path = directory.filePath(QStringLiteral("日本語.sqlite"));
        QSqlDatabase db =
            DatabaseConnection::open({"QSQLITE", "ignored", path, "ignored", "ignored"}, name);
        QVERIFY(db.isValid());
        QVERIFY(db.isOpen());
        QCOMPARE(db.databaseName(), path);
        QVERIFY(db.hostName().isEmpty());
        QVERIFY(db.userName().isEmpty());
        QSqlQuery query(db);
        QVERIFY(query.exec("CREATE TABLE items (value TEXT)"));
        db.close();
        QVERIFY(DatabaseConnection::reopen(db, path));
        QVERIFY(db.tables().contains("items"));
        db.close();
        QVERIFY(!DatabaseConnection::reopen(db, directory.filePath("missing/directory/db.sqlite")));
        QVERIFY(db.lastError().isValid());
    }
    QSqlDatabase::removeDatabase(name);
    const QString missing_driver = QStringLiteral("missing-driver-test");
    {
        QSqlDatabase db = DatabaseConnection::open(
            {"NO_SUCH_DRIVER", "localhost", "db", "user", "secret"}, missing_driver);
        QVERIFY(!db.isValid());
        QVERIFY(!db.isOpen());
    }
    QSqlDatabase::removeDatabase(missing_driver);
}

void TestDatabaseConnection::metadata()
{
    const QByteArray bookmark("bookmark");
    const QJsonObject sqlite = DatabaseConnection::connectionInfo(
        {"QSQLITE", "", "/tmp/日本語.sqlite", "", "secret"}, "SQLite", bookmark);
    QCOMPARE(sqlite["displayName"].toString(), QStringLiteral("SQLite: 日本語.sqlite"));
    QCOMPARE(sqlite["securityScopedBookmark"].toString(), QString::fromLatin1(bookmark.toBase64()));
    QVERIFY(!sqlite.contains("password"));
    const QJsonObject remote = DatabaseConnection::connectionInfo(
        {"QMYSQL", "host", "database", "user", "secret"}, "MySQL", bookmark);
    QCOMPARE(remote["displayName"].toString(), QString("MySQL: user@host/database"));
    QVERIFY(!remote.contains("password"));
    QVERIFY(!remote.contains("securityScopedBookmark"));
}

void TestDatabaseConnection::pathsAndAccess()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("日本語.SQLITE3"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    const QString bad_suffix = directory.filePath("other.txt");
    QFile other(bad_suffix);
    QVERIFY(other.open(QIODevice::WriteOnly));
    other.close();
    QCOMPARE(SQLitePaths::status(path), SQLitePathStatus::Valid);
    QCOMPARE(SQLitePaths::status(bad_suffix), SQLitePathStatus::Unsupported);
    QCOMPARE(SQLitePaths::status(directory.path()), SQLitePathStatus::Missing);
    QCOMPARE(SQLitePaths::status(directory.filePath("missing.db")), SQLitePathStatus::Missing);
    QCOMPARE(SQLitePaths::firstValidLocalFile({QUrl("https://example.test/a.db"),
                                               QUrl::fromLocalFile(bad_suffix),
                                               QUrl::fromLocalFile(path)}),
             QFileInfo(path).absoluteFilePath());
    QVERIFY(SQLitePaths::firstValidLocalFile({QUrl::fromLocalFile(bad_suffix)}).isEmpty());
    const auto access = SQLiteFileAccess::restore("invalid bookmark", path);
    QVERIFY(access != nullptr);
    QCOMPARE(QFileInfo(access->filePath()).canonicalFilePath(),
             QFileInfo(path).canonicalFilePath());
    QVERIFY(SQLiteFileAccess::restore({}, {}).get() == nullptr);
}

QTEST_GUILESS_MAIN(TestDatabaseConnection)
#include "test_database_connection.moc"
