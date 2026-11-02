#include "database_operations.h"

#include "features/db_tool/core/database_connection.h"
#include "features/db_tool/core/query_service.h"
#include "features/web_frontend/gui/bridge/operations/operation_support.h"

#include <QMetaObject>
#include <QMetaType>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThread>
#include <QUuid>
#include <QVariantList>

#include <functional>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {

namespace {

using DatabaseResult = FrontendOperationService::Result;
using DatabaseCallback = std::function<DatabaseResult(const QSqlDatabase &)>;

class DatabaseSession final
{
public:
    DatabaseSession()
    {
        worker_ = new QObject();
        worker_->moveToThread(&thread_);
        QObject::connect(&thread_, &QThread::finished, worker_, &QObject::deleteLater);
        thread_.setObjectName(QStringLiteral("DatabaseSession"));
        thread_.start();
    }

    ~DatabaseSession()
    {
        close();
        thread_.quit();
        thread_.wait();
    }

    DatabaseResult connect(const DatabaseParameters &parameters, const DatabaseCallback &callback)
    {
        return dispatch([this, parameters, callback]() {
            const QString candidateName =
                QStringLiteral("react-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
            QSqlDatabase candidate = DatabaseConnection::open(parameters, candidateName);
            if (!candidate.isOpen()) {
                const QString message = candidate.lastError().text();
                closeConnection(candidate, candidateName);
                return DatabaseResult{{}, QStringLiteral("IO_ERROR"), message, false};
            }

            DatabaseResult result = callback(candidate);
            if (!result.errorCode.isEmpty() || result.cancelled) {
                closeConnection(candidate, candidateName);
                return result;
            }

            QSqlDatabase previous = database_;
            const QString previousName = connection_name_;
            database_ = candidate;
            connection_name_ = candidateName;
            candidate = QSqlDatabase();
            closeConnection(previous, previousName);
            return result;
        });
    }

    DatabaseResult execute(const DatabaseCallback &callback)
    {
        return dispatch([this, callback]() {
            if (!database_.isOpen()) {
                return invalidInput(QStringLiteral("Connect to a database first."));
            }
            return callback(database_);
        });
    }

    void close()
    {
        dispatch([this]() {
            closeConnection(database_, connection_name_);
            connection_name_.clear();
            return true;
        });
    }

private:
    template <typename Function>
    auto dispatch(Function function) -> decltype(function())
    {
        using Result = decltype(function());
        if (QThread::currentThread() == worker_->thread()) {
            return function();
        }

        Result result{};
        const bool invoked = QMetaObject::invokeMethod(
            worker_, [&function, &result]() { result = function(); }, Qt::BlockingQueuedConnection);
        Q_ASSERT(invoked);
        return result;
    }

    static void closeConnection(QSqlDatabase &database, const QString &connectionName)
    {
        if (connectionName.isEmpty()) {
            return;
        }
        database.close();
        database = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
    }

    QThread thread_;
    QObject *worker_{nullptr};
    QSqlDatabase database_;
    QString connection_name_;
};

DatabaseSession &databaseSession()
{
    static DatabaseSession session;
    return session;
}

bool readDatabaseParameters(const QVariantMap &parameters, DatabaseParameters *dbParameters,
                            QString *error)
{
    const QString type = parameters.value(QStringLiteral("type")).toString();
    if (type != QStringLiteral("QSQLITE") && type != QStringLiteral("QMYSQL") &&
        type != QStringLiteral("QPSQL")) {
        *error = QStringLiteral("Choose SQLite, MySQL or PostgreSQL.");
        return false;
    }
    *dbParameters = {type, parameters.value(QStringLiteral("host")).toString(),
                     parameters.value(QStringLiteral("database")).toString(),
                     parameters.value(QStringLiteral("username")).toString(),
                     parameters.value(QStringLiteral("password")).toString()};
    if (dbParameters->database.isEmpty() ||
        (type != QStringLiteral("QSQLITE") &&
         (dbParameters->host.isEmpty() || dbParameters->username.isEmpty()))) {
        *error = QStringLiteral("Complete the database connection fields.");
        return false;
    }
    return true;
}

DatabaseResult withDatabase(const QVariantMap &payload, const DatabaseCallback &callback)
{
    DatabaseParameters parameters;
    QString error;
    if (!readDatabaseParameters(payload.value(QStringLiteral("connection")).toMap(), &parameters,
                                &error)) {
        return invalidInput(error);
    }
    return databaseSession().execute(callback);
}

} // namespace

FrontendOperationService::Result databaseConnect(const QVariantMap &payload)
{
    QVariantMap connection = payload;
    connection.remove(QStringLiteral("action"));
    connection.remove(QStringLiteral("contractVersion"));
    connection.remove(QStringLiteral("requestId"));
    DatabaseParameters parameters;
    QString error;
    if (!readDatabaseParameters(connection, &parameters, &error)) {
        return invalidInput(error);
    }
    return databaseSession().connect(
        parameters,
        [&connection](const QSqlDatabase &database) -> FrontendOperationService::Result {
            const QString displayName = connection.value(QStringLiteral("displayName")).toString();
            const QStringList tables = database.tables();
            return {{{QStringLiteral("connected"), true},
                     {QStringLiteral("displayName"), displayName},
                     {QStringLiteral("tables"), QVariant::fromValue(tables)}},
                    {},
                    {},
                    false};
        });
}

void databaseDisconnect()
{
    databaseSession().close();
}

FrontendOperationService::Result databaseTables(const QVariantMap &payload)
{
    return withDatabase(payload,
                        [](const QSqlDatabase &database) -> FrontendOperationService::Result {
                            QVariantList tables;
                            for (const QString &table : database.tables()) {
                                tables.append(table);
                            }
                            return {{{QStringLiteral("tables"), tables}}, {}, {}, false};
                        });
}

FrontendOperationService::Result databaseQuery(const QVariantMap &payload)
{
    QString sql;
    if (!readText(payload, QStringLiteral("sql"), &sql) || sql.trimmed().isEmpty()) {
        return invalidInput(QStringLiteral("Enter SQL no larger than 1 MiB."));
    }
    return withDatabase(payload, [&sql](const QSqlDatabase &database) {
        QueryExecution execution = QueryService::execute(database, sql);
        if (!execution.success) {
            return FrontendOperationService::Result{
                {}, QStringLiteral("INVALID_INPUT"), execution.error.text(), false};
        }
        QVariantList columns;
        const QSqlRecord record = execution.query.record();
        for (int column = 0; column < record.count(); ++column) {
            columns.append(record.fieldName(column));
        }
        QVariantList rows;
        qsizetype totalBytes = 0;
        bool truncated = false;
        constexpr int MAX_ROWS = 1000;
        constexpr qsizetype MAX_RESULT_BYTES = qsizetype{4} * 1024 * 1024;
        constexpr qsizetype MAX_CELL_TEXT_CHARS = qsizetype{64} * 1024;
        while (execution.query.next()) {
            if (rows.size() >= MAX_ROWS) {
                truncated = true;
                break;
            }
            QVariantList row;
            row.reserve(record.count());
            for (int column = 0; column < record.count(); ++column) {
                const QVariant value = execution.query.value(column);
                if (!value.isValid() || value.isNull()) {
                    row.append(QVariant());
                    continue;
                }
                const QString text = value.toString();
                totalBytes += text.toUtf8().size();
                if (text.size() > MAX_CELL_TEXT_CHARS || totalBytes > MAX_RESULT_BYTES) {
                    return FrontendOperationService::Result{
                        {},
                        QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                        QStringLiteral(
                            "Database results are limited to 4 MiB; reduce the query result."),
                        false};
                }
                row.append(text);
            }
            rows.append(QVariant::fromValue(row));
        }
        return FrontendOperationService::Result{
            {{QStringLiteral("columns"), columns},
             {QStringLiteral("rows"), rows},
             {QStringLiteral("truncated"), truncated},
             {QStringLiteral("affectedRows"), execution.query.numRowsAffected()}},
            {},
            {},
            false};
    });
}

} // namespace FrontendOperationHandlers
