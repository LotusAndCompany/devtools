#include "features/db_tool/core/connection_history.h"
#include "features/web_frontend/core/operations/frontend_operation_service.h"
#include "features/web_frontend/gui/handlers/database_state_handler.h"
#include "features/web_frontend/gui/native_bridge.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QJsonObject>
#include <QSettings>

#include <memory>

void NativeBridge::startDatabaseOperation(const QString &requestId, const QString &operation,
                                          const QVariantMap &payload)
{
    if (operation == QStringLiteral("databaseHistory")) {
        const QSettings settings;
        const QList<QJsonObject> stored = ConnectionHistory::read(settings);
        QVariantList history;
        for (int index = 0; index < stored.size(); ++index) {
            const QJsonObject item = stored.at(index);
            const QString type = item.value(QStringLiteral("type")).toString();
            const QString database = item.value(QStringLiteral("database")).toString();
            const QString displayName = item.value(QStringLiteral("displayName")).toString();
            history.append(QVariantMap{
                {QStringLiteral("index"), index},
                {QStringLiteral("type"), type},
                {QStringLiteral("host"), item.value(QStringLiteral("host")).toString()},
                {QStringLiteral("database"),
                 type == QStringLiteral("QSQLITE") ? QFileInfo(database).fileName() : database},
                {QStringLiteral("username"), item.value(QStringLiteral("username")).toString()},
                {QStringLiteral("displayName"), displayName}});
        }
        completeOnUiThread(
            requestId, {{QStringLiteral("history"), history},
                        {QStringLiteral("selectedFileName"), database_state_->selectedFileName()},
                        {QStringLiteral("connected"), database_state_->connected()}});
        return;
    }
    if (operation == QStringLiteral("databaseDisconnect")) {
        database_state_->disconnect();
        completeOnUiThread(requestId, {{QStringLiteral("disconnected"), true}});
        return;
    }
    if (operation == QStringLiteral("databaseSelectFile")) {
        auto *dialog =
            new QFileDialog(dialog_parent_, tr("Select Database File"), QString(),
                            tr("SQLite Database (*.db *.sqlite *.sqlite3);;All Files (*)"));
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setFileMode(QFileDialog::ExistingFile);
        auto it = operations_.find(requestId);
        if (it == operations_.end()) {
            dialog->deleteLater();
            return;
        }
        it->dialog = dialog;
        connect(dialog, &QFileDialog::accepted, this, [this, dialog, requestId]() {
            auto current = operations_.find(requestId);
            if (current == operations_.end()) {
                return;
            }
            const QString path = dialog->selectedFiles().value(0);
            current->dialog = nullptr;
            if (path.isEmpty() ||
                QFileInfo(path).suffix().compare(QStringLiteral("db"), Qt::CaseInsensitive) != 0 &&
                    QFileInfo(path).suffix().compare(QStringLiteral("sqlite"),
                                                     Qt::CaseInsensitive) != 0 &&
                    QFileInfo(path).suffix().compare(QStringLiteral("sqlite3"),
                                                     Qt::CaseInsensitive) != 0) {
                operations_.erase(current);
                emit operationCancelled(requestId);
                return;
            }
            if (!database_state_->selectFile(path)) {
                fail(requestId, QStringLiteral("PERMISSION_DENIED"),
                     QStringLiteral("Could not access the selected database file."));
                operations_.remove(requestId);
                return;
            }
            completeOnUiThread(requestId,
                               {{QStringLiteral("selected"), true},
                                {QStringLiteral("fileName"), database_state_->selectedFileName()}});
        });
        connect(dialog, &QFileDialog::rejected, this, [this, requestId]() {
            auto current = operations_.find(requestId);
            if (current != operations_.end()) {
                operations_.erase(current);
                emit operationCancelled(requestId);
            }
        });
        dialog->open();
        return;
    }
    if (operation == QStringLiteral("databaseConnect")) {
        QVariantMap connection;
        int historyIndex = -1;
        bool historyIndexValid = false;
        if (payload.contains(QStringLiteral("historyIndex"))) {
            historyIndex = payload.value(QStringLiteral("historyIndex")).toInt(&historyIndexValid);
            const QSettings settings;
            const QList<QJsonObject> stored = ConnectionHistory::read(settings);
            if (!historyIndexValid || historyIndex < 0 || historyIndex >= stored.size()) {
                fail(requestId, QStringLiteral("INVALID_INPUT"),
                     QStringLiteral("Select a valid saved connection."));
                operations_.remove(requestId);
                return;
            }
            connection = stored.at(historyIndex).toVariantMap();
            const QString type = connection.value(QStringLiteral("type")).toString();
            if (type == QStringLiteral("QSQLITE")) {
                const QByteArray bookmark = QByteArray::fromBase64(
                    connection.value(QStringLiteral("securityScopedBookmark"))
                        .toString()
                        .toLatin1());
                if (!database_state_->restoreFileAccess(
                        bookmark, connection.value(QStringLiteral("database")).toString())) {
                    fail(
                        requestId, QStringLiteral("PERMISSION_DENIED"),
                        QStringLiteral("The saved SQLite file permission is no longer available."));
                    operations_.remove(requestId);
                    return;
                }
                connection.insert(QStringLiteral("database"), database_state_->selectedFilePath());
            } else {
                connection.insert(QStringLiteral("password"),
                                  payload.value(QStringLiteral("password")).toString());
            }
        } else {
            connection = payload;
            const QString type = connection.value(QStringLiteral("type")).toString();
            if (type == QStringLiteral("QSQLITE") ||
                (type.isEmpty() && !database_state_->selectedFilePath().isEmpty())) {
                if (!database_state_->fileAccess() ||
                    database_state_->selectedFilePath().isEmpty()) {
                    fail(requestId, QStringLiteral("INVALID_INPUT"),
                         QStringLiteral("Choose a SQLite file before connecting."));
                    operations_.remove(requestId);
                    return;
                }
                connection.insert(QStringLiteral("type"), QStringLiteral("QSQLITE"));
                connection.insert(QStringLiteral("database"), database_state_->selectedFilePath());
                connection.insert(QStringLiteral("host"), QString());
                connection.insert(QStringLiteral("username"), QString());
                connection.insert(QStringLiteral("password"), QString());
                connection.insert(
                    QStringLiteral("displayName"),
                    QStringLiteral("SQLite: %1").arg(database_state_->selectedFileName()));
            }
        }
        const QString type = connection.value(QStringLiteral("type")).toString();
        if (type != QStringLiteral("QSQLITE") && type != QStringLiteral("QMYSQL") &&
            type != QStringLiteral("QPSQL")) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("Choose SQLite, MySQL or PostgreSQL."));
            operations_.remove(requestId);
            return;
        }
        const auto access = database_state_->fileAccess();
        const QString typeText = type == QStringLiteral("QSQLITE")  ? tr("SQLite")
                                 : type == QStringLiteral("QMYSQL") ? tr("MySQL")
                                                                    : tr("PostgreSQL");
        startWorker(
            requestId, operation,
            [connection, access](const CancellationToken &token, const ProgressCallback &progress) {
                Q_UNUSED(access);
                const FrontendOperationService::Result result = FrontendOperationService::execute(
                    QStringLiteral("databaseConnect"), connection, token, progress);
                if (!result.errorCode.isEmpty() || result.cancelled) {
                    return OperationResult{result.value, result.errorCode, result.errorMessage,
                                           result.cancelled};
                }
                return OperationResult{result.value, {}, {}, false};
            },
            [this, connection, typeText](QVariantMap &) {
                database_state_->completeConnection(connection, typeText);
            });
        return;
    }
    if (operation == QStringLiteral("databaseQuery") ||
        operation == QStringLiteral("databaseTables")) {
        if (!database_state_->connected()) {
            fail(requestId, QStringLiteral("INVALID_INPUT"),
                 QStringLiteral("Connect to a database first."));
            operations_.remove(requestId);
            return;
        }
        QVariantMap requestPayload = payload;
        requestPayload.insert(QStringLiteral("connection"), database_state_->connection());
        const auto access = database_state_->fileAccess();
        startWorker(requestId, operation,
                    [operation, requestPayload, access](const CancellationToken &token,
                                                        const ProgressCallback &progress) {
                        Q_UNUSED(access);
                        const FrontendOperationService::Result result =
                            FrontendOperationService::execute(operation, requestPayload, token,
                                                              progress);
                        return OperationResult{result.value, result.errorCode, result.errorMessage,
                                               result.cancelled};
                    });
        return;
    }
    fail(requestId, QStringLiteral("INVALID_INPUT"),
         QStringLiteral("The database operation is not available."));
    operations_.remove(requestId);
}

void NativeBridge::openDroppedSQLiteFile(const QString &path)
{
    const QFileInfo file(path);
    const QString suffix = file.suffix().toLower();
    if (!file.isFile() || (suffix != QStringLiteral("db") && suffix != QStringLiteral("sqlite") &&
                           suffix != QStringLiteral("sqlite3"))) {
        return;
    }
    if (!database_state_->selectFile(file.absoluteFilePath())) {
        return;
    }
    database_state_->clearConnectionForDroppedFile();
    emit navigationRequested(QStringLiteral("db"));
}
