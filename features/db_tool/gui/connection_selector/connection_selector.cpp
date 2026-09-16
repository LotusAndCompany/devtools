#include "connection_selector.h"

#include "features/framework/gui/design_system.h"
#include "features/framework/gui/icon_utils.h"

#include <QEvent>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSqlError>
#include <QStyle>
#include <QVBoxLayout>

ConnectionSelector::ConnectionSelector(QWidget *parent) : QWidget(parent)
{
    buildUi();

    connect(historyListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int const row = historyListWidget->row(item);
        handleHistoryItemClicked(row);
    });
    connect(newConnectionButton, &QPushButton::clicked, this,
            &ConnectionSelector::handleNewConnectionButtonClicked);
    connect(closeButton, &QPushButton::clicked, this,
            &ConnectionSelector::handleCloseButtonClicked);

    loadHistory();
}

void ConnectionSelector::buildUi()
{
    DevTools::Ui::configureDialog(this);

    auto *verticalLayout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(verticalLayout);

    history_group_box = DevTools::Ui::createPane(QString(), this);
    history_group_box->setFlat(true);
    auto *historyLayout = new QVBoxLayout(history_group_box);
    DevTools::Ui::applyPanelLayout(historyLayout);
    historyListWidget = new QListWidget(history_group_box);
    DevTools::Ui::configureItemView(historyListWidget);
    historyLayout->addWidget(historyListWidget);
    verticalLayout->addWidget(history_group_box, 1);

    auto *buttonLayout = new QHBoxLayout();

    newConnectionButton = new QPushButton(this);
    DevTools::Ui::configureCompactButton(newConnectionButton);
    buttonLayout->addWidget(newConnectionButton);

    closeButton = new QPushButton(this);
    DevTools::Ui::configureCompactButton(closeButton);
    buttonLayout->addWidget(closeButton);
    DevTools::Ui::configureActionBar(buttonLayout, DevTools::Ui::ActionBarAlignment::Trailing);

    verticalLayout->addLayout(buttonLayout);

    retranslateUi();
}

void ConnectionSelector::retranslateUi()
{
    setWindowTitle(tr("DB Connection"));
    history_group_box->setTitle(tr("Connection History"));
    newConnectionButton->setText(tr("New Connection"));
    closeButton->setText(tr("Close"));
}

void ConnectionSelector::loadHistory()
{
    QSettings const settings;
    QStringList const historyList = settings.value("db_tool/connectionHistory").toStringList();

    connectionHistory.clear();
    for (const QString &jsonStr : historyList) {
        QJsonDocument const doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (!doc.isNull() && doc.isObject()) {
            connectionHistory.append(doc.object());
        }
    }

    refreshHistoryList();
}

void ConnectionSelector::refreshHistoryList()
{
    historyListWidget->clear();

    for (int i = 0; i < connectionHistory.size(); ++i) {
        const QJsonObject &conn = connectionHistory[i];
        QString const displayName = conn["displayName"].toString();

        auto *itemWidget = new QWidget();
        itemWidget->setFont(DevTools::Ui::standardFont());
        auto *layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(2 * DevTools::Ui::Metrics::LIST_ROW_HORIZONTAL_INSET, 0,
                                   2 * DevTools::Ui::Metrics::LIST_ROW_HORIZONTAL_INSET, 0);
        layout->setSpacing(DevTools::Ui::Metrics::COMPACT_SPACING);

        auto *label = new QLabel(displayName);
        label->setFont(DevTools::Ui::standardFont());
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        auto *deleteButton = new QPushButton();
        DevTools::Ui::configureListActionButton(deleteButton, QStringLiteral("delete"),
                                                tr("Delete"));
        deleteButton->setIcon(
            IconUtils::themedIcon(QStringLiteral("delete"), QStyle::SP_TrashIcon));
        deleteButton->setProperty("historyIndex", i);

        connect(deleteButton, &QPushButton::clicked, this, [this, i]() { removeHistoryItem(i); });

        layout->addWidget(label, 1);
        layout->addWidget(deleteButton);

        auto *item = new QListWidgetItem();
        DevTools::Ui::configureListItem(item);
        historyListWidget->addItem(item);
        historyListWidget->setItemWidget(item, itemWidget);
    }
}

void ConnectionSelector::handleHistoryItemClicked(int row)
{
    if (row < 0 || row >= connectionHistory.size()) {
        return;
    }

    const QJsonObject &conn = connectionHistory[row];
    if (connectWithPassword(conn)) {
        close();
    }
}

bool ConnectionSelector::connectWithPassword(const QJsonObject &connectionInfo)
{
    QString const dbType = connectionInfo["type"].toString();
    QString const hostName = connectionInfo["host"].toString();
    QString const databaseName = connectionInfo["database"].toString();
    QString const userName = connectionInfo["username"].toString();

    QString password;

    // SQLite以外はパスワードを入力
    if (dbType != "QSQLITE") {
        bool ok;
        password = QInputDialog::getText(
            this, tr("Password"),
            tr("Enter password for %1:").arg(connectionInfo["displayName"].toString()),
            QLineEdit::Password, QString(), &ok);
        if (!ok) {
            return false;
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(dbType);

    if (dbType != "QSQLITE") {
        db.setHostName(hostName);
        db.setDatabaseName(databaseName);
        db.setUserName(userName);
        db.setPassword(password);
    } else {
        db.setDatabaseName(databaseName);
    }

    if (!db.open()) {
        QMessageBox::critical(this, tr("Connection Failed"), db.lastError().text());
        return false;
    }

    QMessageBox::information(this, tr("Success"), tr("Database connection established."));
    emit connectionCreated(db);
    return true;
}

void ConnectionSelector::removeHistoryItem(int index)
{
    if (index < 0 || index >= connectionHistory.size()) {
        return;
    }

    connectionHistory.removeAt(index);

    // Save updated history to settings
    QSettings settings;
    QStringList historyList;
    for (const QJsonObject &conn : connectionHistory) {
        QJsonDocument const doc(conn);
        historyList.append(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    }
    settings.setValue("db_tool/connectionHistory", historyList);

    refreshHistoryList();
}

void ConnectionSelector::handleNewConnectionButtonClicked()
{
    emit newConnectionRequested();
}

void ConnectionSelector::handleCloseButtonClicked()
{
    close();
}

void ConnectionSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else {
        QWidget::changeEvent(event);
    }
}
