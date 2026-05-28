#include "connection_window.h"

#include <QComboBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlError>
#include <QUrl>
#include <QVBoxLayout>

namespace {
constexpr int DEFAULT_WIDTH = 621;
constexpr int DEFAULT_HEIGHT = 600;
constexpr int FIXED_HEIGHT_FULL = 600;
constexpr int FIXED_HEIGHT_SQLITE = 300;
constexpr int LAYOUT_SPACING = 25;

bool isSQLiteFilePath(const QString &filePath)
{
    QFileInfo const fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    const QString suffix = fileInfo.suffix().toLower();
    return suffix == "db" || suffix == "sqlite" || suffix == "sqlite3";
}

QString droppedSQLiteFilePath(const QMimeData *mimeData)
{
    if (mimeData == nullptr || !mimeData->hasUrls()) {
        return {};
    }

    for (const QUrl &url : mimeData->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        const QString filePath = url.toLocalFile();
        if (isSQLiteFilePath(filePath)) {
            return QFileInfo(filePath).absoluteFilePath();
        }
    }

    return {};
}
} // namespace

ConnectionWindow::ConnectionWindow(QWidget *parent) : QWidget(parent)
{
    buildUi();

    ConnectionWindow::init();

    connect(ConnectPushButton, &QPushButton::clicked, this, &ConnectionWindow::createNewConnect);
    connect(ClosePushButton, &QPushButton::clicked, this, &QWidget::close);
    connect(dbTypeComboBox, &QComboBox::currentIndexChanged, this,
            &ConnectionWindow::selectedDBType);
    connect(browseButton, &QPushButton::clicked, this, &ConnectionWindow::browseForDatabase);

    dbNamelineEdit->installEventFilter(this);
}

void ConnectionWindow::buildUi()
{
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    QFont baseFont = font();
    baseFont.setFamily(".AppleSystemUIFont");
    setFont(baseFont);

    setAutoFillBackground(false);

    auto *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(LAYOUT_SPACING);

    dbTypeLabel = new QLabel(this);
    verticalLayout->addWidget(dbTypeLabel);

    dbTypeComboBox = new QComboBox(this);
    dbTypeComboBox->addItem(QString());
    verticalLayout->addWidget(dbTypeComboBox);

    hostNameLabel = new QLabel(this);
    verticalLayout->addWidget(hostNameLabel);

    hostNameLineEdit = new QLineEdit(this);
    verticalLayout->addWidget(hostNameLineEdit);

    dbNameLabel = new QLabel(this);
    verticalLayout->addWidget(dbNameLabel);

    auto *dbNameLayout = new QHBoxLayout();
    dbNamelineEdit = new QLineEdit(this);
    dbNameLayout->addWidget(dbNamelineEdit);
    browseButton = new QPushButton(this);
    dbNameLayout->addWidget(browseButton);
    verticalLayout->addLayout(dbNameLayout);

    userNameLabel = new QLabel(this);
    verticalLayout->addWidget(userNameLabel);

    userNameLineEdit = new QLineEdit(this);
    verticalLayout->addWidget(userNameLineEdit);

    passwordLabel = new QLabel(this);
    verticalLayout->addWidget(passwordLabel);

    passwordLineEdit = new QLineEdit(this);
    verticalLayout->addWidget(passwordLineEdit);

    auto *buttonLayout = new QHBoxLayout();
    ConnectPushButton = new QPushButton(this);
    buttonLayout->addWidget(ConnectPushButton);
    ClosePushButton = new QPushButton(this);
    buttonLayout->addWidget(ClosePushButton);
    verticalLayout->addLayout(buttonLayout);

    retranslateUi();
}

void ConnectionWindow::retranslateUi()
{
    setWindowTitle(tr("New Connection"));
    dbTypeLabel->setText(tr("Database Type"));
    if (dbTypeComboBox->count() > 0) {
        dbTypeComboBox->setItemText(0, tr("SQLite"));
    }
    hostNameLabel->setText(tr("Host Name"));
    dbNameLabel->setText(tr("Database Name"));
    browseButton->setText(tr("Browse..."));
    userNameLabel->setText(tr("User Name"));
    passwordLabel->setText(tr("Password"));
    ConnectPushButton->setText(tr("Connect"));
    ClosePushButton->setText(tr("Close"));
}

void ConnectionWindow::init()
{
    ConnectionWindow::selectedDBType();
}

void ConnectionWindow::selectedDBType()
{
    QString const dbTypeText = dbTypeComboBox->currentText();
    bool const isSQLite = (dbTypeText == tr("SQLite"));
    bool const display = !isSQLite;

    if (display) {
        this->setFixedHeight(FIXED_HEIGHT_FULL);
    } else {
        this->setFixedHeight(FIXED_HEIGHT_SQLITE);
    }

    hostNameLabel->setVisible(display);
    hostNameLineEdit->setVisible(display);
    userNameLabel->setVisible(display);
    userNameLineEdit->setVisible(display);
    passwordLabel->setVisible(display);
    passwordLineEdit->setVisible(display);

    // Show browse button only for SQLite
    browseButton->setVisible(isSQLite);
    dbNamelineEdit->setAcceptDrops(isSQLite);
}

void ConnectionWindow::browseForDatabase()
{
    QString const filePath = QFileDialog::getOpenFileName(
        this, tr("Select Database File"), QString(),
        tr("SQLite Database (*.db *.sqlite *.sqlite3);;All Files (*)"));

    if (!filePath.isEmpty()) {
        dbNamelineEdit->setText(filePath);
    }
}

void ConnectionWindow::createNewConnect()
{
    // get param from ui input
    const QString hostName = hostNameLineEdit->text();
    const QString databaseName = dbNamelineEdit->text();
    const QString userName = userNameLineEdit->text();
    const QString password = passwordLineEdit->text();

    QString databaseType;
    QString const dbTypeText = dbTypeComboBox->currentText();

    if (dbTypeText == tr("SQLite")) {
        databaseType = "QSQLITE";
    } else if (dbTypeText == "MySQL") {
        databaseType = "QMYSQL";
    } else if (dbTypeText == "PostgreSQL") {
        databaseType = "QPSQL";
    }
    if (databaseType.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Database type not supported."));
        return;
    }

    if (databaseType != "QSQLITE" && (hostName.isEmpty() || databaseName.isEmpty() ||
                                      userName.isEmpty() || password.isEmpty())) {
        QMessageBox::warning(this, tr("Error"), tr("Some fields are missing."));
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(databaseType);

    if (databaseType != "QSQLITE") {
        db.setHostName(hostName);
        db.setDatabaseName(databaseName);
        db.setUserName(userName);
        db.setPassword(password);
    } else {
        db.setDatabaseName(databaseName);
    }

    if (!db.open()) {
        QMessageBox::critical(this, tr("Connection Failed"), db.lastError().text());
        return;
    }

    // Build connection info for history
    QString displayName;
    if (databaseType == "QSQLITE") {
        QFileInfo const fileInfo(databaseName);
        displayName = QString("SQLite: %1").arg(fileInfo.fileName());
    } else {
        displayName =
            QString("%1: %2@%3/%4").arg(dbTypeText).arg(userName).arg(hostName).arg(databaseName);
    }

    lastConnectionInfo = QJsonObject{{"type", databaseType},
                                     {"host", hostName},
                                     {"database", databaseName},
                                     {"username", userName},
                                     {"displayName", displayName}};

    QMessageBox::information(this, tr("Success"), tr("Database connection established."));
    emit connectionCreated(db, lastConnectionInfo);
    close();
}

QJsonObject ConnectionWindow::getConnectionInfo() const
{
    return lastConnectionInfo;
}

void ConnectionWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else {
        QWidget::changeEvent(event);
    }
}

bool ConnectionWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != dbNamelineEdit || dbTypeComboBox->currentText() != tr("SQLite")) {
        return QWidget::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::DragEnter:
    case QEvent::DragMove: {
        auto *dragEvent = dynamic_cast<QDragMoveEvent *>(event);
        if (dragEvent == nullptr) {
            return QWidget::eventFilter(watched, event);
        }

        if (!droppedSQLiteFilePath(dragEvent->mimeData()).isEmpty()) {
            dragEvent->acceptProposedAction();
        } else {
            dragEvent->ignore();
        }
        return true;
    }
    case QEvent::Drop: {
        auto *dropEvent = dynamic_cast<QDropEvent *>(event);
        if (dropEvent == nullptr) {
            return QWidget::eventFilter(watched, event);
        }

        const QString filePath = droppedSQLiteFilePath(dropEvent->mimeData());
        if (filePath.isEmpty()) {
            dropEvent->ignore();
            return true;
        }

        dbNamelineEdit->setText(filePath);
        dropEvent->acceptProposedAction();
        return true;
    }
    default:
        return QWidget::eventFilter(watched, event);
    }
}
