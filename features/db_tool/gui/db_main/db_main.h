#ifndef DB_MAIN_H
#define DB_MAIN_H

#include "../../core/sqlite_file_access.h"
#include "QtSql/qsqldatabase.h"

#include <QJsonObject>
#include <QListWidgetItem>
#include <QWidget>

class ConnectionSelector;
class ConnectionWindow;
class QGroupBox;
class QListWidget;
class QPushButton;
class QTabWidget;

class dbMain : public QWidget
{
    Q_OBJECT

public:
    explicit dbMain(QWidget *parent = nullptr);
    ~dbMain() override = default;
    dbMain(const dbMain &) = delete;
    dbMain &operator=(const dbMain &) = delete;
    dbMain(dbMain &&) = delete;
    dbMain &operator=(dbMain &&) = delete;

    /**
     * @brief SQLiteファイルへ接続する
     * @param filePath SQLiteファイルパス
     * @return 接続できた場合はtrue
     */
    bool connectSQLiteFile(const QString &filePath);

private:
    void buildUi();
    void retranslateUi();
    void setDatabase(const QSqlDatabase &database, const QJsonObject &connectionInfo,
                     std::shared_ptr<SQLiteFileAccess> newFileAccess);

    QPushButton *refreshTableButton{nullptr};
    QPushButton *connectionSettingsButton{nullptr};
    QPushButton *addQueryTabButton{nullptr};
    QGroupBox *toolbarGroupBox{nullptr};
    QGroupBox *tablesGroupBox{nullptr};
    QGroupBox *queryGroupBox{nullptr};
    QListWidget *tableListWidget{nullptr};
    QTabWidget *queryTabWidget{nullptr};

    std::shared_ptr<SQLiteFileAccess> sqliteFileAccess;
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
    static void saveConnectionHistory(const QJsonObject &connectionInfo);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void handleTabCloseRequested(int index);
    void handleTableClicked(QListWidgetItem *item);
};

#endif // DB_MAIN_H
