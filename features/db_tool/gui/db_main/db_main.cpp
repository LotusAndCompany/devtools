#include "db_main.h"

#include "../connection_selector/connection_selector.h"
#include "../connection_window/connection_window.h"
#include "../query_page/query_page.h"

#include <QEvent>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTabWidget>
#include <QTableView>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr int DEFAULT_WIDTH = 943;
constexpr int DEFAULT_HEIGHT = 349;
constexpr int CONTENT_STRETCH_TABLES = 1;
constexpr int CONTENT_STRETCH_QUERY = 3;
constexpr int SPACER_WIDTH = 40;
constexpr int SPACER_HEIGHT = 20;
constexpr int MAX_HISTORY_ENTRIES = 10;
} // namespace

// Static variable to track if connection was made during this app session
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool s_hasConnectedThisSession = false;

dbMain::dbMain(QWidget *parent) : QWidget(parent)
{
    buildUi();

    queryTabWidget->setTabsClosable(true);
    connect(queryTabWidget, &QTabWidget::tabCloseRequested, this, &dbMain::handleTabCloseRequested);
    connect(addQueryTabButton, &QPushButton::clicked, this, &dbMain::handleAddQueryTabButtonClick);
    connect(refreshTableButton, &QPushButton::clicked, this,
            &dbMain::handleRefreshTableButtonClick);
    connect(connectionSettingsButton, &QPushButton::clicked, this,
            &dbMain::handleConnectionSettingsButtonClick);

    // Disable buttons until DB is connected
    addQueryTabButton->setEnabled(false);
    refreshTableButton->setEnabled(false);

    // Check if we should show connection dialog on startup
    QTimer::singleShot(0, this, [this]() {
        QSettings const settings;
        QStringList const history = settings.value("db_tool/connectionHistory").toStringList();

        // Show dialog if no history and not connected in this session
        if (history.isEmpty() && !s_hasConnectedThisSession) {
            showConnectionSelector();
        }
    });
}

void dbMain::buildUi()
{
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    auto *verticalLayoutMain = new QVBoxLayout(this);

    toolbarGroupBox = new QGroupBox(this);
    auto *toolbarLayout = new QHBoxLayout(toolbarGroupBox);

    refreshTableButton = new QPushButton(toolbarGroupBox);
    refreshTableButton->setIcon(QIcon::fromTheme("refresh"));
    toolbarLayout->addWidget(refreshTableButton);

    auto *horizontalSpacer =
        new QSpacerItem(SPACER_WIDTH, SPACER_HEIGHT, QSizePolicy::Expanding, QSizePolicy::Minimum);
    toolbarLayout->addItem(horizontalSpacer);

    connectionSettingsButton = new QPushButton(toolbarGroupBox);
    toolbarLayout->addWidget(connectionSettingsButton);

    addQueryTabButton = new QPushButton(toolbarGroupBox);
    toolbarLayout->addWidget(addQueryTabButton);

    verticalLayoutMain->addWidget(toolbarGroupBox);

    auto *contentLayout = new QHBoxLayout();

    tablesGroupBox = new QGroupBox(this);
    auto *tablesLayout = new QVBoxLayout(tablesGroupBox);
    tableListWidget = new QListWidget(tablesGroupBox);
    tablesLayout->addWidget(tableListWidget);
    contentLayout->addWidget(tablesGroupBox, CONTENT_STRETCH_TABLES);

    queryGroupBox = new QGroupBox(this);
    auto *queryLayout = new QVBoxLayout(queryGroupBox);
    queryTabWidget = new QTabWidget(queryGroupBox);
    queryTabWidget->setCurrentIndex(-1);
    queryTabWidget->setTabsClosable(true);
    queryLayout->addWidget(queryTabWidget);
    contentLayout->addWidget(queryGroupBox, CONTENT_STRETCH_QUERY);

    verticalLayoutMain->addLayout(contentLayout);

    retranslateUi();
}

void dbMain::retranslateUi()
{
    setWindowTitle(tr("Form"));
    toolbarGroupBox->setTitle(tr("Toolbar"));
    refreshTableButton->setToolTip(tr("Refresh Tables"));
    connectionSettingsButton->setText(tr("DB Connection Settings"));
    addQueryTabButton->setText(tr("SQL"));
    tablesGroupBox->setTitle(tr("Tables"));
    queryGroupBox->setTitle(tr("Query"));
}

void dbMain::handleAddQueryTabButtonClick()
{
    auto *page = new QueryPage(this);

    QString const baseName = "Query";
    int counter = 1;
    QString newTabName;

    do { // NOLINT(cppcoreguidelines-avoid-do-while)
        newTabName = QString("%1 %2").arg(baseName).arg(counter++);
    } while (isTabNameExists(newTabName));

    queryTabWidget->addTab(page, newTabName);
    queryTabWidget->setCurrentWidget(page); // 追加されたタブに自動で切り替え
}

void dbMain::handleRefreshTableButtonClick()
{
    if (!db.isOpen()) {
        QMessageBox::warning(this, tr("DB Not Connected"), tr("Not connected to database"));
        return;
    }

    // 既存アイテムをクリア
    tableListWidget->clear();

    // 再度テーブル一覧を取得
    populateTableList();
}

bool dbMain::connectSQLiteFile(const QString &filePath)
{
    QFileInfo const fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        QMessageBox::warning(this, tr("Invalid File"), tr("The selected file does not exist."));
        return false;
    }

    const QString suffix = fileInfo.suffix().toLower();
    if (suffix != "db" && suffix != "sqlite" && suffix != "sqlite3") {
        QMessageBox::warning(this, tr("Unsupported File"),
                             tr("Please select a SQLite database file."));
        return false;
    }

    // Create a unique temporary connection name to avoid conflicts
    const QString tempConnectionName = QUuid::createUuid().toString();
    QSqlDatabase sqliteDb = QSqlDatabase::addDatabase("QSQLITE", tempConnectionName);
    sqliteDb.setDatabaseName(fileInfo.absoluteFilePath());

    if (!sqliteDb.open()) {
        QMessageBox::critical(this, tr("Connection Failed"), sqliteDb.lastError().text());
        QSqlDatabase::removeDatabase(tempConnectionName);
        return false;
    }

    QSqlQuery validationQuery(sqliteDb);
    if (!validationQuery.exec("SELECT name FROM sqlite_master WHERE type='table' LIMIT 1;")) {
        QMessageBox::critical(this, tr("Connection Failed"), validationQuery.lastError().text());
        sqliteDb.close();
        QSqlDatabase::removeDatabase(tempConnectionName);
        return false;
    }

    const QJsonObject connectionInfo{
        {"type", "QSQLITE"},
        {"host", ""},
        {"database", fileInfo.absoluteFilePath()},
        {"username", ""},
        {"displayName", QString("SQLite: %1").arg(fileInfo.fileName())},
    };

    setDatabase(sqliteDb, connectionInfo);
    return true;
}

bool dbMain::isTabNameExists(const QString &tabName)
{
    for (int i = 0; i < queryTabWidget->count(); ++i) {
        if (queryTabWidget->tabText(i) == tabName) {
            return true;
        }
    }
    return false;
}

void dbMain::handleTabCloseRequested(int index)
{
    QWidget const *widget = queryTabWidget->widget(index);
    queryTabWidget->removeTab(index);
    delete widget;
}

void dbMain::populateTableList()
{
    QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table';", db);
    while (query.next()) {
        tableListWidget->addItem(query.value(0).toString());
    }
}

void dbMain::handleTableClicked(QListWidgetItem *item)
{
    QString const tableName = item->text();

    // 同名タブがある場合はそれを選択
    for (int i = 0; i < queryTabWidget->count(); ++i) {
        if (queryTabWidget->tabText(i) == tableName) {
            queryTabWidget->setCurrentIndex(i);
            return;
        }
    }

    auto *model = new QSqlTableModel(this, db);
    model->setTable(tableName);
    model->select();

    auto *tableView = new QTableView;
    tableView->setModel(model);

    // 更新ボタン
    auto *refreshButton = new QPushButton;
    refreshButton->setIcon(QIcon::fromTheme("refresh"));
    refreshButton->setToolTip(tr("Refresh"));
    connect(refreshButton, &QPushButton::clicked, this, [model]() { model->select(); });
    // 左寄せのレイアウト
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(buttonLayout); // 更新ボタン
    mainLayout->addWidget(tableView);    // テーブルビュー

    auto *container = new QWidget;
    container->setLayout(mainLayout);
    container->setAttribute(Qt::WA_DeleteOnClose);

    queryTabWidget->addTab(container, tableName);
    queryTabWidget->setCurrentWidget(container);
}

void dbMain::setDatabase(const QSqlDatabase &database, const QJsonObject &connectionInfo)
{
    db = database;
    s_hasConnectedThisSession = true;

    // Enable buttons now that DB is connected
    addQueryTabButton->setEnabled(true);
    refreshTableButton->setEnabled(true);

    // Save connection info to history if provided
    if (!connectionInfo.isEmpty()) {
        saveConnectionHistory(connectionInfo);
    }

    // Clear existing items before populating
    tableListWidget->clear();
    populateTableList();

    // Disconnect previous connections to avoid duplicates
    disconnect(tableListWidget, &QListWidget::itemClicked, this, &dbMain::handleTableClicked);
    connect(tableListWidget, &QListWidget::itemClicked, this, &dbMain::handleTableClicked);
}

void dbMain::handleConnectionSettingsButtonClick()
{
    showConnectionSelector();
}

void dbMain::showConnectionSelector()
{
    if (connectionSelector != nullptr) {
        connectionSelector->raise();
        connectionSelector->activateWindow();
        return;
    }

    connectionSelector = new ConnectionSelector(this);
    connectionSelector->setAttribute(Qt::WA_DeleteOnClose);

    // Inherit window flags from parent, including stay-on-top if set
    Qt::WindowFlags flags = connectionSelector->windowFlags() | Qt::Window;
    if ((window() != nullptr) && ((window()->windowFlags() & Qt::WindowStaysOnTopHint) != 0U)) {
        flags |= Qt::WindowStaysOnTopHint;
    }
    connectionSelector->setWindowFlags(flags);

    connectionSelector->show();
    connectionSelector->raise();
    connectionSelector->activateWindow();

    connect(connectionSelector, &ConnectionSelector::destroyed, this,
            [this]() { connectionSelector = nullptr; });

    connect(connectionSelector, &ConnectionSelector::connectionCreated, this,
            [this](const QSqlDatabase &db) { setDatabase(db); });

    connect(connectionSelector, &ConnectionSelector::newConnectionRequested, this,
            [this]() { openNewConnectionWindow(); });
}

void dbMain::openNewConnectionWindow()
{
    if (connectionWindow != nullptr) {
        connectionWindow->raise();
        connectionWindow->activateWindow();
        return;
    }

    connectionWindow = new ConnectionWindow(this);
    connectionWindow->setAttribute(Qt::WA_DeleteOnClose);

    // Inherit window flags from parent, including stay-on-top if set
    Qt::WindowFlags flags = connectionWindow->windowFlags() | Qt::Window;
    if ((window() != nullptr) && ((window()->windowFlags() & Qt::WindowStaysOnTopHint) != 0U)) {
        flags |= Qt::WindowStaysOnTopHint;
    }
    connectionWindow->setWindowFlags(flags);

    connectionWindow->show();
    connectionWindow->raise();
    connectionWindow->activateWindow();

    connect(connectionWindow, &ConnectionWindow::destroyed, this,
            [this]() { connectionWindow = nullptr; });

    connect(connectionWindow, &ConnectionWindow::connectionCreated, this,
            [this](const QSqlDatabase &db, const QJsonObject &connectionInfo) {
                setDatabase(db, connectionInfo);
                // ConnectionSelectorも閉じる
                if (connectionSelector != nullptr) {
                    connectionSelector->close();
                }
            });
}

void dbMain::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else {
        QWidget::changeEvent(event);
    }
}

void dbMain::saveConnectionHistory(const QJsonObject &connectionInfo)
{
    QSettings settings;
    QStringList historyList = settings.value("db_tool/connectionHistory").toStringList();

    // Convert to JSON string
    QJsonDocument const doc(connectionInfo);
    QString const jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    // Check for duplicates (same database path/name)
    QString const newDatabase = connectionInfo["database"].toString();
    QString const newType = connectionInfo["type"].toString();
    QString const newHost = connectionInfo["host"].toString();

    for (int i = 0; i < historyList.size(); ++i) {
        QJsonDocument const existingDoc = QJsonDocument::fromJson(historyList[i].toUtf8());
        if (!existingDoc.isNull() && existingDoc.isObject()) {
            QJsonObject existing = existingDoc.object();
            if (existing["database"].toString() == newDatabase &&
                existing["type"].toString() == newType && existing["host"].toString() == newHost) {
                // Remove existing entry (will be added at front)
                historyList.removeAt(i);
                break;
            }
        }
    }

    // Add new entry at front
    historyList.prepend(jsonStr);

    // Limit to MAX_HISTORY_ENTRIES entries
    while (historyList.size() > MAX_HISTORY_ENTRIES) {
        historyList.removeLast();
    }

    settings.setValue("db_tool/connectionHistory", historyList);
}
