#include "db_main.h"

#include "../connection_selector/connection_selector.h"
#include "../connection_window/connection_window.h"
#include "../query_page/query_page.h"
#include "ui_db_main.h"

#include <QEvent>
#include <QJsonDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTabWidget>
#include <QTableView>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>
#include <QWidget>

// Static variable to track if connection was made during this app session
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool s_hasConnectedThisSession = false;

dbMain::dbMain(QWidget *parent) : QWidget(parent), ui(new Ui::dbMain)
{
    ui->setupUi(this);

    ui->queryTabWidget->setTabsClosable(true);
    connect(ui->queryTabWidget, &QTabWidget::tabCloseRequested, this,
            &dbMain::handleTabCloseRequested);
    connect(ui->addQueryTabButton, &QPushButton::clicked, this,
            &dbMain::handleAddQueryTabButtonClick);
    connect(ui->refreshTableButton, &QPushButton::clicked, this,
            &dbMain::handleRefreshTableButtonClick);
    connect(ui->connectionSettingsButton, &QPushButton::clicked, this,
            &dbMain::handleConnectionSettingsButtonClick);

    // Disable buttons until DB is connected
    ui->addQueryTabButton->setEnabled(false);
    ui->refreshTableButton->setEnabled(false);

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

void dbMain::handleAddQueryTabButtonClick()
{
    auto *page = new QueryPage(this);

    QString const baseName = "Query";
    int counter = 1;
    QString newTabName;

    do { // NOLINT(cppcoreguidelines-avoid-do-while)
        newTabName = QString("%1 %2").arg(baseName).arg(counter++);
    } while (isTabNameExists(newTabName));

    ui->queryTabWidget->addTab(page, newTabName);
    ui->queryTabWidget->setCurrentWidget(page); // 追加されたタブに自動で切り替え
}

void dbMain::handleRefreshTableButtonClick()
{
    if (!db.isOpen()) {
        QMessageBox::warning(this, tr("DB Not Connected"), tr("Not connected to database"));
        return;
    }

    // 既存アイテムをクリア
    ui->tableListWidget->clear();

    // 再度テーブル一覧を取得
    populateTableList();
}

bool dbMain::isTabNameExists(const QString &tabName)
{
    for (int i = 0; i < ui->queryTabWidget->count(); ++i) {
        if (ui->queryTabWidget->tabText(i) == tabName) {
            return true;
        }
    }
    return false;
}

void dbMain::handleTabCloseRequested(int index)
{
    QWidget const *widget = ui->queryTabWidget->widget(index);
    ui->queryTabWidget->removeTab(index);
    delete widget;
}

void dbMain::populateTableList()
{
    QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table';", db);
    while (query.next()) {
        ui->tableListWidget->addItem(query.value(0).toString());
    }
}

void dbMain::handleTableClicked(QListWidgetItem *item)
{
    QString const tableName = item->text();

    // 同名タブがある場合はそれを選択
    for (int i = 0; i < ui->queryTabWidget->count(); ++i) {
        if (ui->queryTabWidget->tabText(i) == tableName) {
            ui->queryTabWidget->setCurrentIndex(i);
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

    ui->queryTabWidget->addTab(container, tableName);
    ui->queryTabWidget->setCurrentWidget(container);
}

dbMain::~dbMain()
{
    delete ui;
}

void dbMain::setDatabase(const QSqlDatabase &database, const QJsonObject &connectionInfo)
{
    db = database;
    s_hasConnectedThisSession = true;

    // Enable buttons now that DB is connected
    ui->addQueryTabButton->setEnabled(true);
    ui->refreshTableButton->setEnabled(true);

    // Save connection info to history if provided
    if (!connectionInfo.isEmpty()) {
        saveConnectionHistory(connectionInfo);
    }

    // Clear existing items before populating
    ui->tableListWidget->clear();
    populateTableList();

    // Disconnect previous connections to avoid duplicates
    disconnect(ui->tableListWidget, &QListWidget::itemClicked, this, &dbMain::handleTableClicked);
    connect(ui->tableListWidget, &QListWidget::itemClicked, this, &dbMain::handleTableClicked);
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
                if (connectionSelector) {
                    connectionSelector->close();
                }
            });
}

void dbMain::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
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

    // Limit to 10 entries
    while (historyList.size() > 10) {
        historyList.removeLast();
    }

    settings.setValue("db_tool/connectionHistory", historyList);
}
