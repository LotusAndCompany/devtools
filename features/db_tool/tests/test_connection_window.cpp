#include "features/db_tool/gui/connection_window/connection_window.h"

#include <QComboBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

namespace Test {
class TestConnectionWindow : public QObject
{
    Q_OBJECT

private slots:
    void exposesSupportedDatabaseTypes();
    void connectsToAccessibleSQLiteFileWithoutPicker_data();
    void connectsToAccessibleSQLiteFileWithoutPicker();
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
void TestConnectionWindow::connectsToAccessibleSQLiteFileWithoutPicker_data()
{
    QTest::addColumn<bool>("dropFile");
    QTest::newRow("typed path") << false;
    QTest::newRow("dropped path") << true;
}

void TestConnectionWindow::connectsToAccessibleSQLiteFileWithoutPicker()
{
    QFETCH(bool, dropFile);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("test.sqlite");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    ConnectionWindow window;
    auto *databaseField = window.findChild<QLineEdit *>(QStringLiteral("databaseNameLineEdit"));
    QVERIFY(databaseField != nullptr);
    if (dropFile) {
        QMimeData mimeData;
        mimeData.setUrls({QUrl::fromLocalFile(path)});
        QDragEnterEvent enter(QPoint(1, 1), Qt::CopyAction, &mimeData, Qt::LeftButton,
                              Qt::NoModifier);
        QApplication::sendEvent(databaseField, &enter);
        QVERIFY(enter.isAccepted());
        QDropEvent drop(QPointF(1, 1), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(databaseField, &drop);
        QVERIFY(drop.isAccepted());
    } else {
        databaseField->setText(path);
    }
    QCOMPARE(databaseField->text(), path);

    bool connected = false;
    QString connectedPath;
    connect(&window, &ConnectionWindow::connectionCreated, &window,
            [&](QSqlDatabase db, const QJsonObject &info) {
                connected = db.isOpen();
                connectedPath = info["database"].toString();
            });
    bool pickerShown = false;
    QTimer dismissDialogs;
    connect(&dismissDialogs, &QTimer::timeout, &window, [&] {
        for (auto *widget : QApplication::topLevelWidgets()) {
            if (auto *picker = qobject_cast<QFileDialog *>(widget)) {
                pickerShown = true;
                picker->reject();
            } else if (auto *message = qobject_cast<QMessageBox *>(widget)) {
                message->accept();
            }
        }
    });
    dismissDialogs.start(10);
    QPushButton *connectButton = nullptr;
    for (auto *button : window.findChildren<QPushButton *>()) {
        if (button->text() == "Connect") {
            connectButton = button;
            break;
        }
    }
    QVERIFY(connectButton != nullptr);
    connectButton->click();
    QSqlDatabase::database().close();
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    QVERIFY(!pickerShown);
    QVERIFY(connected);
    QCOMPARE(connectedPath, path);
}

} // namespace Test

QTEST_MAIN(Test::TestConnectionWindow)

#include "test_connection_window.moc"
