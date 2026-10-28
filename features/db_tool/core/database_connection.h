#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include <QJsonObject>
#include <QSqlDatabase>
#include <QString>

struct DatabaseParameters
{
    QString type;
    QString host;
    QString database;
    QString username;
    QString password;
};

class DatabaseConnection
{
public:
    static QSqlDatabase open(const DatabaseParameters &parameters,
                             const QString &connection_name = QSqlDatabase::defaultConnection);
    static bool reopen(QSqlDatabase &database, const QString &database_name);
    static QJsonObject connectionInfo(const DatabaseParameters &parameters,
                                      const QString &type_text, const QByteArray &bookmark);
};

#endif // DATABASE_CONNECTION_H
