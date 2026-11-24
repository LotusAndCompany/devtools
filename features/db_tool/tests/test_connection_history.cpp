#include "features/db_tool/core/connection_history.h"

#include <QJsonDocument>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

class TestConnectionHistory : public QObject
{
    Q_OBJECT

private slots:
    void preservesHistoryFormat();
};

void TestConnectionHistory::preservesHistoryFormat()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QSettings settings(directory.filePath("settings.ini"), QSettings::IniFormat);
    QVERIFY(ConnectionHistory::isEmpty(settings));
    const QJsonObject first{
        {"type", "QSQLITE"}, {"database", "日本語.sqlite"}, {"host", ""}, {"unknown", "kept"}};
    ConnectionHistory::add(settings, first);
    QCOMPARE(ConnectionHistory::read(settings), QList<QJsonObject>({first}));

    QStringList raw = settings.value("db_tool/connectionHistory").toStringList();
    raw.prepend("invalid-json");
    settings.setValue("db_tool/connectionHistory", raw);
    QVERIFY(!ConnectionHistory::isEmpty(settings));
    QCOMPARE(ConnectionHistory::read(settings), QList<QJsonObject>({first}));

    QJsonObject updated = first;
    updated["displayName"] = "new";
    ConnectionHistory::add(settings, updated);
    raw = settings.value("db_tool/connectionHistory").toStringList();
    QCOMPARE(raw.size(), 2);
    QCOMPARE(raw[1], QString("invalid-json"));
    QCOMPARE(QJsonDocument::fromJson(raw[0].toUtf8()).object(), updated);
    QCOMPARE(ConnectionHistory::read(settings).first()["unknown"].toString(), QString("kept"));

    for (int i = 0; i < 12; ++i) {
        ConnectionHistory::add(settings, {{"type", "QSQLITE"}, {"database", QString::number(i)}});
    }
    raw = settings.value("db_tool/connectionHistory").toStringList();
    QCOMPARE(raw.size(), 10);
    QCOMPARE(ConnectionHistory::read(settings).first()["database"].toString(), QString("11"));
    ConnectionHistory::write(settings, {updated});
    settings.sync();
    QCOMPARE(settings.status(), QSettings::NoError);
    QSettings reloaded(directory.filePath("settings.ini"), QSettings::IniFormat);
    QCOMPARE(ConnectionHistory::read(reloaded), QList<QJsonObject>({updated}));
}

QTEST_GUILESS_MAIN(TestConnectionHistory)
#include "test_connection_history.moc"
