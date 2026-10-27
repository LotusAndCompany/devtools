#ifndef SQLITE_PATHS_H
#define SQLITE_PATHS_H

#include <QList>
#include <QString>
#include <QUrl>

enum class SQLitePathStatus { Valid, Missing, Unsupported };

class SQLitePaths
{
public:
    static SQLitePathStatus status(const QString &path);
    static QString firstValidLocalFile(const QList<QUrl> &urls);
};

#endif // SQLITE_PATHS_H
