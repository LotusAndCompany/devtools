#include "sqlite_paths.h"

#include "features/framework/core/local_file_selection_service.h"

#include <QFileInfo>

namespace {
const QStringList &sqliteSuffixes()
{
    static const QStringList suffixes{QStringLiteral("db"), QStringLiteral("sqlite"),
                                      QStringLiteral("sqlite3")};
    return suffixes;
}
} // namespace

SQLitePathStatus SQLitePaths::status(const QString &path)
{
    const QFileInfo file_info(path);
    if (!file_info.exists() || !file_info.isFile()) {
        return SQLitePathStatus::Missing;
    }
    return LocalFileSelectionService::hasSuffix(path, sqliteSuffixes())
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
    return LocalFileSelectionService::firstExistingLocalFile(urls, sqliteSuffixes());
}
