#include "connection_window.h"

#include "ui_connection_window.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QSqlDatabase>
#include <QSqlError>
#include <QUrl>

namespace {
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

ConnectionWindow::ConnectionWindow(QWidget *parent) : QWidget(parent), ui(new Ui::ConnectionWindow)
{
    ui->setupUi(this);

    ConnectionWindow::init();

    connect(ui->ConnectPushButton, &QPushButton::clicked, this,
            &ConnectionWindow::createNewConnect);
    connect(ui->ClosePushButton, &QPushButton::clicked, this, &QWidget::close);
    connect(ui->dbTypeComboBox, &QComboBox::currentIndexChanged, this,
            &ConnectionWindow::selectedDBType);
    connect(ui->browseButton, &QPushButton::clicked, this, &ConnectionWindow::browseForDatabase);

    ui->dbNamelineEdit->installEventFilter(this);
}

ConnectionWindow::~ConnectionWindow()
{
    delete ui;
}

void ConnectionWindow::init()
{
    ConnectionWindow::selectedDBType();
}

void ConnectionWindow::selectedDBType()
{
    QString const dbTypeText = ui->dbTypeComboBox->currentText();
    bool const isSQLite = (dbTypeText == "SQLite");
    bool const display = !isSQLite;

    if (display) {
        this->setFixedHeight(600);
    } else {
        this->setFixedHeight(300);
    }

    ui->hostNameLabel->setVisible(display);
    ui->hostNameLineEdit->setVisible(display);
    ui->userNameLabel->setVisible(display);
    ui->userNameLineEdit->setVisible(display);
    ui->passwordLabel->setVisible(display);
    ui->passwordLineEdit->setVisible(display);

    // Show browse button only for SQLite
    ui->browseButton->setVisible(isSQLite);
    ui->dbNamelineEdit->setAcceptDrops(isSQLite);
}

void ConnectionWindow::browseForDatabase()
{
    QString const filePath = QFileDialog::getOpenFileName(
        this, tr("Select Database File"), QString(),
        tr("SQLite Database (*.db *.sqlite *.sqlite3);;All Files (*)"));

    if (!filePath.isEmpty()) {
        ui->dbNamelineEdit->setText(filePath);
    }
}

void ConnectionWindow::createNewConnect()
{
    // get param from ui input
    const QString hostName = ui->hostNameLineEdit->text();
    const QString databaseName = ui->dbNamelineEdit->text();
    const QString userName = ui->userNameLineEdit->text();
    const QString password = ui->passwordLineEdit->text();

    QString databaseType;
    QString const dbTypeText = ui->dbTypeComboBox->currentText();

    if (dbTypeText == "SQLite") {
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
    } else {
        // Build connection info for history
        QString displayName;
        if (databaseType == "QSQLITE") {
            QFileInfo const fileInfo(databaseName);
            displayName = QString("SQLite: %1").arg(fileInfo.fileName());
        } else {
            displayName = QString("%1: %2@%3/%4")
                              .arg(dbTypeText)
                              .arg(userName)
                              .arg(hostName)
                              .arg(databaseName);
        }

        lastConnectionInfo = QJsonObject{{"type", databaseType},
                                         {"host", hostName},
                                         {"database", databaseName},
                                         {"username", userName},
                                         {"displayName", displayName}};

        QMessageBox::information(this, tr("Success"), tr("Database connection established."));
        emit connectionCreated(db, lastConnectionInfo);
        close();
        return;
    }
}

QJsonObject ConnectionWindow::getConnectionInfo() const
{
    return lastConnectionInfo;
}

void ConnectionWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    } else {
        QWidget::changeEvent(event);
    }
}

bool ConnectionWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != ui->dbNamelineEdit || ui->dbTypeComboBox->currentText() != "SQLite") {
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

        ui->dbNamelineEdit->setText(filePath);
        dropEvent->acceptProposedAction();
        return true;
    }
    default:
        return QWidget::eventFilter(watched, event);
    }
}
