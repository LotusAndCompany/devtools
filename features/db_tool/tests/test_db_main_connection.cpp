#include "features/db_tool/gui/db_main/db_main.h"

#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

class TestDbMainConnection : public QObject
{
    Q_OBJECT

private slots:
    void connectAndValidateFiles();
};

void TestDbMainConnection::connectAndValidateFiles()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QCoreApplication::setOrganizationName("devtools-test");
    QCoreApplication::setApplicationName("db-main-connection-test");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory.path());
    const QStringList original_connections = QSqlDatabase::connectionNames();
    const QString path = directory.filePath(QStringLiteral("日本語.sqlite"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    const QString other_path = directory.filePath("other.txt");
    QFile other(other_path);
    QVERIFY(other.open(QIODevice::WriteOnly));
    other.close();
    {
        dbMain widget;
        QVERIFY(widget.connectSQLiteFile(path));
        QString message;
        QTimer timer;
        connect(&timer, &QTimer::timeout, &widget, [&] {
            for (auto *window : QApplication::topLevelWidgets()) {
                if (auto *box = qobject_cast<QMessageBox *>(window)) {
                    message = box->text();
                    box->accept();
                }
            }
        });
        timer.start(10);
        QVERIFY(!widget.connectSQLiteFile(directory.filePath("missing.sqlite")));
        QCOMPARE(message, QString("The selected file does not exist."));
        QVERIFY(!widget.connectSQLiteFile(other_path));
        QCOMPARE(message, QString("Please select a SQLite database file."));
    }
    for (const QString &name : QSqlDatabase::connectionNames()) {
        if (!original_connections.contains(name)) {
            {
                QSqlDatabase db = QSqlDatabase::database(name, false);
                db.close();
            }
            QSqlDatabase::removeDatabase(name);
        }
    }
}

QTEST_MAIN(TestDbMainConnection)
#include "test_db_main_connection.moc"
