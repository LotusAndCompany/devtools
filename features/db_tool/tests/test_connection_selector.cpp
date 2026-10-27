#include "features/db_tool/gui/connection_selector/connection_selector.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QListWidget>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

class TestConnectionSelector : public QObject
{
    Q_OBJECT

private slots:
    void reconnectsFromHistory();
};

void TestConnectionSelector::reconnectsFromHistory()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QCoreApplication::setOrganizationName("devtools-test");
    QCoreApplication::setApplicationName("connection-selector-test");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory.path());
    const QString path = directory.filePath(QStringLiteral("履歴.sqlite"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    QJsonObject history{{"type", "QSQLITE"},
                        {"database", path},
                        {"displayName", "test"},
                        {"securityScopedBookmark", "aW52YWxpZA=="},
                        {"unknown", "preserved"}};
    QSettings settings;
    settings.setValue(
        "db_tool/connectionHistory",
        QStringList{QString::fromUtf8(QJsonDocument(history).toJson(QJsonDocument::Compact))});
    bool connected = false;
    QJsonObject result;
    std::shared_ptr<SQLiteFileAccess> access;
    {
        ConnectionSelector selector;
        auto *list = selector.findChild<QListWidget *>();
        QVERIFY(list != nullptr);
        QCOMPARE(list->count(), 1);
        connect(&selector, &ConnectionSelector::connectionCreated, &selector,
                [&](QSqlDatabase db, const QJsonObject &info,
                    std::shared_ptr<SQLiteFileAccess> file_access) {
                    connected = db.isOpen();
                    result = info;
                    access = std::move(file_access);
                });
        QTimer timer;
        connect(&timer, &QTimer::timeout, &selector, [] {
            for (auto *window : QApplication::topLevelWidgets()) {
                if (auto *box = qobject_cast<QMessageBox *>(window)) {
                    box->accept();
                }
            }
        });
        timer.start(10);
        QVERIFY(QMetaObject::invokeMethod(&selector, "handleHistoryItemClicked", Q_ARG(int, 0)));
        QVERIFY(connected);
        QVERIFY(access != nullptr);
        QCOMPARE(QFileInfo(result["database"].toString()).canonicalFilePath(),
                 QFileInfo(path).canonicalFilePath());
        QCOMPARE(result["unknown"].toString(), QString("preserved"));
    }
    access.reset();
    {
        QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
        db.close();
    }
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
}

QTEST_MAIN(TestConnectionSelector)
#include "test_connection_selector.moc"
