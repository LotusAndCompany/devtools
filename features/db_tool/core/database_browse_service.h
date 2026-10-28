#ifndef DATABASE_BROWSE_SERVICE_H
#define DATABASE_BROWSE_SERVICE_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>

class QObject;
class QSqlTableModel;

class DatabaseBrowseService
{
public:
    static QSqlError validateSQLiteDatabase(const QSqlDatabase &database);
    static QStringList tableNames(const QSqlDatabase &database);
    static QSqlTableModel *tableModel(const QSqlDatabase &database, const QString &tableName,
                                      QObject *parent = nullptr);
};

#endif // DATABASE_BROWSE_SERVICE_H
