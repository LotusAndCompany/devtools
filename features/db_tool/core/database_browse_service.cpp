#include "database_browse_service.h"

#include <QSqlQuery>
#include <QSqlTableModel>

QSqlError DatabaseBrowseService::validateSQLiteDatabase(const QSqlDatabase &database)
{
    QSqlQuery query(database);
    if (!query.exec("SELECT name FROM sqlite_master WHERE type='table' LIMIT 1;")) {
        return query.lastError();
    }
    return {};
}

QStringList DatabaseBrowseService::tableNames(const QSqlDatabase &database)
{
    return database.tables(QSql::Tables);
}

QSqlTableModel *DatabaseBrowseService::tableModel(const QSqlDatabase &database,
                                                  const QString &tableName, QObject *parent)
{
    auto *model = new QSqlTableModel(parent, database);
    model->setTable(tableName);
    model->select();
    return model;
}
