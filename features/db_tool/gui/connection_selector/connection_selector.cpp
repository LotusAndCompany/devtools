#include "connection_selector.h"

#include "features/framework/gui/icon_utils.h"

#include <QEvent>
#include <QFont>
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
#include <QSizePolicy>
#include <QSpacerItem>
#include <QSqlError>
#include <QStyle>
#include <QVBoxLayout>

namespace {
constexpr int DEFAULT_WIDTH = 400;
constexpr int DEFAULT_HEIGHT = 300;
constexpr int DELETE_BUTTON_SIZE = 24;
constexpr int ITEM_MARGIN_HORIZONTAL = 5;
constexpr int ITEM_MARGIN_VERTICAL = 2;
constexpr int SPACER_WIDTH = 40;
constexpr int SPACER_HEIGHT = 20;
} // namespace

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
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    auto *verticalLayout = new QVBoxLayout(this);

    titleLabel = new QLabel(this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    verticalLayout->addWidget(titleLabel);

    historyListWidget = new QListWidget(this);
    historyListWidget->setAlternatingRowColors(true);
    verticalLayout->addWidget(historyListWidget);

    auto *buttonLayout = new QHBoxLayout();
    auto *horizontalSpacer =
        new QSpacerItem(SPACER_WIDTH, SPACER_HEIGHT, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout->addItem(horizontalSpacer);

    newConnectionButton = new QPushButton(this);
    buttonLayout->addWidget(newConnectionButton);

    closeButton = new QPushButton(this);
    buttonLayout->addWidget(closeButton);

    verticalLayout->addLayout(buttonLayout);

    retranslateUi();
}

void ConnectionSelector::retranslateUi()
{
    setWindowTitle(tr("DB Connection"));
    titleLabel->setText(tr("Connection History"));
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
        auto *layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(ITEM_MARGIN_HORIZONTAL, ITEM_MARGIN_VERTICAL,
                                   ITEM_MARGIN_HORIZONTAL, ITEM_MARGIN_VERTICAL);

        auto *label = new QLabel(displayName);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        auto *deleteButton = new QPushButton();
        deleteButton->setIcon(
            IconUtils::themedIcon(QStringLiteral("delete"), QStyle::SP_TrashIcon));
        deleteButton->setFixedSize(DELETE_BUTTON_SIZE, DELETE_BUTTON_SIZE);
        deleteButton->setToolTip(tr("Delete"));
        deleteButton->setProperty("historyIndex", i);

        connect(deleteButton, &QPushButton::clicked, this, [this, i]() { removeHistoryItem(i); });

        layout->addWidget(label);
        layout->addWidget(deleteButton);

        auto *item = new QListWidgetItem();
        item->setSizeHint(itemWidget->sizeHint());
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
