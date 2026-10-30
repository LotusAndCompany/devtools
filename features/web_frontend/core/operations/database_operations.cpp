#include "database_operations.h"

#include "features/db_tool/core/database_connection.h"
#include "features/db_tool/core/query_service.h"
#include "operation_support.h"

#include <QMetaType>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>
#include <QVariantList>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {

FrontendOperationService::Result withDatabase(
    const QVariantMap &payload,
    const std::function<FrontendOperationService::Result(const QSqlDatabase &)> &callback)
{
    const QVariantMap parameters = payload.value(QStringLiteral("connection")).toMap();
    const QString type = parameters.value(QStringLiteral("type")).toString();
    if (type != QStringLiteral("QSQLITE") && type != QStringLiteral("QMYSQL") &&
        type != QStringLiteral("QPSQL")) {
        return invalidInput(QStringLiteral("Choose SQLite, MySQL or PostgreSQL."));
    }
    const DatabaseParameters dbParameters{type, parameters.value(QStringLiteral("host")).toString(),
                                          parameters.value(QStringLiteral("database")).toString(),
                                          parameters.value(QStringLiteral("username")).toString(),
                                          parameters.value(QStringLiteral("password")).toString()};
    if (dbParameters.database.isEmpty() ||
        (type != QStringLiteral("QSQLITE") &&
         (dbParameters.host.isEmpty() || dbParameters.username.isEmpty()))) {
        return invalidInput(QStringLiteral("Complete the database connection fields."));
    }
    const QString connectionName =
        QStringLiteral("react-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    QSqlDatabase database = DatabaseConnection::open(dbParameters, connectionName);
    if (!database.isOpen()) {
        const QString message = database.lastError().text();
        database = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        return {{}, QStringLiteral("IO_ERROR"), message, false};
    }
    FrontendOperationService::Result result = callback(database);
    database.close();
    database = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
    return result;
}

FrontendOperationService::Result databaseConnect(const QVariantMap &payload)
{
    QVariantMap connection = payload;
    connection.remove(QStringLiteral("action"));
    connection.remove(QStringLiteral("contractVersion"));
    connection.remove(QStringLiteral("requestId"));
    const QVariantMap wrapper{{QStringLiteral("connection"), connection}};
    return withDatabase(
        wrapper, [&connection](const QSqlDatabase &database) -> FrontendOperationService::Result {
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
        constexpr qsizetype MAX_RESULT_BYTES = 4 * 1024 * 1024;
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
                if (text.size() > 64 * 1024 || totalBytes > MAX_RESULT_BYTES) {
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
