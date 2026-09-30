#include "connection_window.h"

#include "../sqlite_file_picker.h"
#include "features/framework/gui/design_system.h"

#include <QComboBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
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

#include <utility>

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

QJsonObject buildConnectionInfo(const QString &databaseType, const QString &dbTypeText,
                                const QString &hostName, const QString &databaseName,
                                const QString &userName, const SQLiteFileAccess *sqliteFileAccess)
{
    QString displayName;
    if (databaseType == "QSQLITE") {
        QFileInfo const fileInfo(databaseName);
        displayName = QString("SQLite: %1").arg(fileInfo.fileName());
    } else {
        displayName =
            QString("%1: %2@%3/%4").arg(dbTypeText).arg(userName).arg(hostName).arg(databaseName);
    }

    QJsonObject connectionInfo{{"type", databaseType},
                               {"host", hostName},
                               {"database", databaseName},
                               {"username", userName},
                               {"displayName", displayName}};
    if (databaseType == "QSQLITE" && sqliteFileAccess != nullptr &&
        !sqliteFileAccess->bookmarkData().isEmpty()) {
        connectionInfo["securityScopedBookmark"] =
            QString::fromLatin1(sqliteFileAccess->bookmarkData().toBase64());
    }
    return connectionInfo;
}

bool prepareSQLiteFileAccess(QWidget *parent, const QString &databaseName,
                             std::unique_ptr<SQLiteFileAccess> &sqliteFileAccess)
{
    if (!sqliteFileAccess || sqliteFileAccess->filePath() != databaseName) {
        sqliteFileAccess = SQLiteFileAccess::fromFilePath(databaseName);
    }
    if (!sqliteFileAccess) {
        sqliteFileAccess = selectSQLiteDatabaseFile(parent, databaseName);
    }
    if (!sqliteFileAccess) {
        return false;
    }
    return true;
}

bool openDatabase(QSqlDatabase &db, const QString &databaseType, QWidget *parent,
                  QLineEdit *databaseNameLineEdit, const QString &databaseName,
                  std::unique_ptr<SQLiteFileAccess> &sqliteFileAccess,
                  const QString &connectionFailedTitle)
{
    if (!db.open()) {
#ifdef Q_OS_MACOS
        if (databaseType == "QSQLITE") {
            db.close();
            auto selectedFile = selectSQLiteDatabaseFile(parent, databaseName);
            if (!selectedFile) {
                return false;
            }
            sqliteFileAccess = std::move(selectedFile);
            const QString selectedDatabaseName = sqliteFileAccess->filePath();
            databaseNameLineEdit->setText(selectedDatabaseName);
            db.setDatabaseName(selectedDatabaseName);
            db.open();
        }
#else
        Q_UNUSED(databaseType);
        Q_UNUSED(databaseNameLineEdit);
        Q_UNUSED(databaseName);
        Q_UNUSED(sqliteFileAccess);
#endif
        if (!db.isOpen()) {
            QMessageBox::critical(parent, connectionFailedTitle, db.lastError().text());
            return false;
        }
    }
    return true;
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
    DevTools::Ui::configureDialog(this);

    auto *verticalLayout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(verticalLayout);

    form_group_box = DevTools::Ui::createPane(QString(), this);
    DevTools::Ui::configureCompactPane(form_group_box);
    auto *formGroupLayout = new QVBoxLayout(form_group_box);
    DevTools::Ui::applyPanelLayout(formGroupLayout);

    auto *formLayout = new QFormLayout;
    DevTools::Ui::configureInlineFormLayout(formLayout);
    buildFormFields(formLayout);
    formGroupLayout->addLayout(formLayout);
    verticalLayout->addWidget(form_group_box);
    verticalLayout->addStretch();

    buildActionButtons(verticalLayout);

    retranslateUi();
}

void ConnectionWindow::buildFormFields(QFormLayout *formLayout)
{
    dbTypeLabel = new QLabel(this);
    dbTypeComboBox = new QComboBox(this);
    dbTypeComboBox->setObjectName(QStringLiteral("databaseTypeComboBox"));
    dbTypeComboBox->addItem(QString(), QStringLiteral("QSQLITE"));
    dbTypeComboBox->addItem(QString(), QStringLiteral("QMYSQL"));
    dbTypeComboBox->addItem(QString(), QStringLiteral("QPSQL"));
    DevTools::Ui::configureComboBox(dbTypeComboBox);
    formLayout->addRow(dbTypeLabel, dbTypeComboBox);

    hostNameLabel = new QLabel(this);
    hostNameLineEdit = new QLineEdit(this);
    DevTools::Ui::configureLineEdit(hostNameLineEdit);
    formLayout->addRow(hostNameLabel, hostNameLineEdit);

    dbNameLabel = new QLabel(this);
    auto *dbNameLayout = new QHBoxLayout();
    DevTools::Ui::applyInlineLayout(dbNameLayout);
    dbNamelineEdit = new QLineEdit(this);
    dbNamelineEdit->setObjectName(QStringLiteral("databaseNameLineEdit"));
    DevTools::Ui::configureLineEdit(dbNamelineEdit);
    dbNameLayout->addWidget(dbNamelineEdit);
    browseButton = new QPushButton(this);
    DevTools::Ui::configureCompactButton(browseButton);
    dbNameLayout->addWidget(browseButton);
    formLayout->addRow(dbNameLabel, dbNameLayout);

    userNameLabel = new QLabel(this);
    userNameLineEdit = new QLineEdit(this);
    DevTools::Ui::configureLineEdit(userNameLineEdit);
    formLayout->addRow(userNameLabel, userNameLineEdit);

    passwordLabel = new QLabel(this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    DevTools::Ui::configureLineEdit(passwordLineEdit);
    formLayout->addRow(passwordLabel, passwordLineEdit);
}

void ConnectionWindow::buildActionButtons(QVBoxLayout *verticalLayout)
{
    auto *buttonLayout = new QHBoxLayout();
    ConnectPushButton = new QPushButton(this);
    DevTools::Ui::configureCompactButton(ConnectPushButton);
    buttonLayout->addWidget(ConnectPushButton);
    ClosePushButton = new QPushButton(this);
    DevTools::Ui::configureCompactButton(ClosePushButton);
    buttonLayout->addWidget(ClosePushButton);
    DevTools::Ui::configureActionBar(buttonLayout, DevTools::Ui::ActionBarAlignment::Trailing);
    verticalLayout->addLayout(buttonLayout);
}

void ConnectionWindow::retranslateUi()
{
    setWindowTitle(tr("New Connection"));
    form_group_box->setTitle(tr("New Connection"));
    dbTypeLabel->setText(tr("Database Type"));
    const QStringList databaseTypeNames = {tr("SQLite"), tr("MySQL"), tr("PostgreSQL")};
    for (int i = 0; i < databaseTypeNames.size() && i < dbTypeComboBox->count(); ++i) {
        dbTypeComboBox->setItemText(i, databaseTypeNames[i]);
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
    bool const isSQLite = (dbTypeComboBox->currentData().toString() == "QSQLITE");
    bool const display = !isSQLite;

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
    std::unique_ptr<SQLiteFileAccess> selectedFile =
        selectSQLiteDatabaseFile(this, dbNamelineEdit->text());
    if (selectedFile) {
        dbNamelineEdit->setText(selectedFile->filePath());
        sqliteFileAccess = std::move(selectedFile);
    }
}

void ConnectionWindow::createNewConnect()
{
    // get param from ui input
    const QString hostName = hostNameLineEdit->text();
    QString databaseName = dbNamelineEdit->text();
    const QString userName = userNameLineEdit->text();
    const QString password = passwordLineEdit->text();

    QString const dbTypeText = dbTypeComboBox->currentText();
    QString const databaseType = dbTypeComboBox->currentData().toString();
    if (databaseType.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Database type not supported."));
        return;
    }

    if (databaseType != "QSQLITE" && (hostName.isEmpty() || databaseName.isEmpty() ||
                                      userName.isEmpty() || password.isEmpty())) {
        QMessageBox::warning(this, tr("Error"), tr("Some fields are missing."));
        return;
    }

    if (databaseType == "QSQLITE") {
        if (!prepareSQLiteFileAccess(this, databaseName, sqliteFileAccess)) {
            return;
        }
        databaseName = sqliteFileAccess->filePath();
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

    if (!openDatabase(db, databaseType, this, dbNamelineEdit, databaseName, sqliteFileAccess,
                      tr("Connection Failed"))) {
        return;
    }
    if (databaseType == "QSQLITE") {
        databaseName = sqliteFileAccess->filePath();
    }

    lastConnectionInfo = buildConnectionInfo(databaseType, dbTypeText, hostName, databaseName,
                                             userName, sqliteFileAccess.get());

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
    if (watched != dbNamelineEdit || dbTypeComboBox->currentData().toString() != "QSQLITE") {
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

        sqliteFileAccess = SQLiteFileAccess::fromFilePath(filePath);
        dbNamelineEdit->setText(sqliteFileAccess ? sqliteFileAccess->filePath() : filePath);
        dropEvent->acceptProposedAction();
        return true;
    }
    default:
        return QWidget::eventFilter(watched, event);
    }
}
