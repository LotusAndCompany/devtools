#include "sqlite_paths.h"

#include <QFileInfo>

SQLitePathStatus SQLitePaths::status(const QString &path)
{
    const QFileInfo file_info(path);
    if (!file_info.exists() || !file_info.isFile()) {
        return SQLitePathStatus::Missing;
    }
    const QString suffix = file_info.suffix().toLower();
    return suffix == "db" || suffix == "sqlite" || suffix == "sqlite3"
               ? SQLitePathStatus::Valid
               : SQLitePathStatus::Unsupported;
}

QString SQLitePaths::firstValidLocalFile(const QList<QUrl> &urls)
{
    const QString path = firstValidFilePath(urls);
    return path.isEmpty() ? QString() : QFileInfo(path).absoluteFilePath();
}

QString SQLitePaths::firstValidFilePath(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
        if (url.isLocalFile() && status(url.toLocalFile()) == SQLitePathStatus::Valid) {
            return url.toLocalFile();
        }
    }
    return {};
}
