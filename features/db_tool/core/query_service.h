#ifndef QUERY_SERVICE_H
#define QUERY_SERVICE_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

struct QueryExecution
{
    QSqlQuery query;
    QSqlError error;
    bool success;
};

class QueryService
{
public:
    static QueryExecution execute(const QSqlDatabase &database, const QString &sql);
};

#endif // QUERY_SERVICE_H
