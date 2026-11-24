#include "query_service.h"

#include <utility>

QueryExecution QueryService::execute(const QSqlDatabase &database, const QString &sql)
{
    QSqlQuery query(database);
    const bool success = query.exec(sql);
    const QSqlError error = query.lastError();
    return {std::move(query), error, success};
}
