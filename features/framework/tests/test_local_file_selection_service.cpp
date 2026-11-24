#include "features/framework/core/local_file_selection_service.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

class TestLocalFileSelectionService : public QObject
{
    Q_OBJECT

private slots:
    void checksSuffixCaseInsensitively();
    void selectsFirstExistingLocalFile();
};

void TestLocalFileSelectionService::checksSuffixCaseInsensitively()
{
    QVERIFY(LocalFileSelectionService::hasSuffix(
        QStringLiteral("database.SQLITE3"),
        {QStringLiteral("db"), QStringLiteral("sqlite"), QStringLiteral("sqlite3")}));
    QVERIFY(!LocalFileSelectionService::hasSuffix(
        QStringLiteral("database.txt"),
        {QStringLiteral("db"), QStringLiteral("sqlite"), QStringLiteral("sqlite3")}));
}

void TestLocalFileSelectionService::selectsFirstExistingLocalFile()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString unsupportedPath = directory.filePath(QStringLiteral("notes.txt"));
    QFile unsupported(unsupportedPath);
    QVERIFY(unsupported.open(QIODevice::WriteOnly));
    unsupported.close();
    const QString expectedPath = directory.filePath(QStringLiteral("primary.DB"));
    QFile expected(expectedPath);
    QVERIFY(expected.open(QIODevice::WriteOnly));
    expected.close();
    const QString secondPath = directory.filePath(QStringLiteral("secondary.sqlite"));
    QFile second(secondPath);
    QVERIFY(second.open(QIODevice::WriteOnly));
    second.close();
    const QString directoryPath = directory.filePath(QStringLiteral("folder.sqlite"));
    QVERIFY(QDir().mkdir(directoryPath));

    const QString selected = LocalFileSelectionService::firstExistingLocalFile(
        {QUrl(QStringLiteral("https://example.test/remote.db")),
         QUrl::fromLocalFile(unsupportedPath),
         QUrl::fromLocalFile(directory.filePath(QStringLiteral("missing.db"))),
         QUrl::fromLocalFile(directoryPath), QUrl::fromLocalFile(expectedPath),
         QUrl::fromLocalFile(secondPath)},
        {QStringLiteral("db"), QStringLiteral("sqlite"), QStringLiteral("sqlite3")});

    QCOMPARE(selected, expectedPath);
}

QTEST_GUILESS_MAIN(TestLocalFileSelectionService)
#include "test_local_file_selection_service.moc"
