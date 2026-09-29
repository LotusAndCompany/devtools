#include "features/db_tool/gui/connection_window/connection_window.h"

#include <QComboBox>
#include <QtTest>

namespace Test {
class TestConnectionWindow : public QObject
{
    Q_OBJECT

private slots:
    void exposesSupportedDatabaseTypes();
};

void TestConnectionWindow::exposesSupportedDatabaseTypes()
{
    ConnectionWindow window;
    auto *const combo = window.findChild<QComboBox *>(QStringLiteral("databaseTypeComboBox"));
    QVERIFY(combo != nullptr);

    QCOMPARE(combo->count(), 3);
    QCOMPARE(combo->itemText(0), QStringLiteral("SQLite"));
    QCOMPARE(combo->itemText(1), QStringLiteral("MySQL"));
    QCOMPARE(combo->itemText(2), QStringLiteral("PostgreSQL"));
    QCOMPARE(combo->itemData(0).toString(), QStringLiteral("QSQLITE"));
    QCOMPARE(combo->itemData(1).toString(), QStringLiteral("QMYSQL"));
    QCOMPARE(combo->itemData(2).toString(), QStringLiteral("QPSQL"));
}
} // namespace Test

QTEST_MAIN(Test::TestConnectionWindow)

#include "test_connection_window.moc"
