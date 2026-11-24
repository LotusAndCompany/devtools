#include "database_connection.h"

#include <QFileInfo>

QSqlDatabase DatabaseConnection::open(const DatabaseParameters &parameters,
                                      const QString &connection_name)
{
    QSqlDatabase database = QSqlDatabase::addDatabase(parameters.type, connection_name);
    if (parameters.type != "QSQLITE") {
        database.setHostName(parameters.host);
        database.setDatabaseName(parameters.database);
        database.setUserName(parameters.username);
        database.setPassword(parameters.password);
    } else {
        database.setDatabaseName(parameters.database);
    }
    database.open();
    return database;
}

bool DatabaseConnection::reopen(QSqlDatabase &database, const QString &database_name)
{
    database.setDatabaseName(database_name);
    return database.open();
}

QJsonObject DatabaseConnection::connectionInfo(const DatabaseParameters &parameters,
                                               const QString &type_text, const QByteArray &bookmark)
{
    QString display_name;
    if (parameters.type == "QSQLITE") {
        display_name = QString("SQLite: %1").arg(QFileInfo(parameters.database).fileName());
    } else {
        display_name = QString("%1: %2@%3/%4")
                           .arg(type_text)
                           .arg(parameters.username)
                           .arg(parameters.host)
                           .arg(parameters.database);
    }
    QJsonObject info{{"type", parameters.type},
                     {"host", parameters.host},
                     {"database", parameters.database},
                     {"username", parameters.username},
                     {"displayName", display_name}};
    if (parameters.type == "QSQLITE" && !bookmark.isEmpty()) {
        info["securityScopedBookmark"] = QString::fromLatin1(bookmark.toBase64());
    }
    return info;
}
