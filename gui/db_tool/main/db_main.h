#ifndef DB_MAIN_H
#define DB_MAIN_H

#include "QtSql/qsqldatabase.h"

#include <QJsonObject>
#include <QListWidgetItem>
#include <QWidget>

namespace Ui {
class dbMain;
}

class ConnectionSelector;
class ConnectionWindow;

class dbMain : public QWidget
{
    Q_OBJECT

public:
    explicit dbMain(QWidget *parent = nullptr);
    ~dbMain();

private:
    Ui::dbMain *ui;
    QSqlDatabase db;
    ConnectionSelector *connectionSelector = nullptr;
    ConnectionWindow *connectionWindow = nullptr;

    void handleAddQueryTabButtonClick();
    void handleRefreshTableButtonClick();
    void handleConnectionSettingsButtonClick();
    bool isTabNameExists(const QString &tabName);
    void populateTableList();
    void showConnectionSelector();
    void openNewConnectionWindow();
    void saveConnectionHistory(const QJsonObject &connectionInfo);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void handleTabCloseRequested(int index);
    void handleTableClicked(QListWidgetItem *item);
    void setDatabase(const QSqlDatabase &database,
                     const QJsonObject &connectionInfo = QJsonObject());
};

#endif // DB_MAIN_H
