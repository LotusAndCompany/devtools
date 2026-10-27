#include "sqlite_file_access.h"

std::unique_ptr<SQLiteFileAccess> SQLiteFileAccess::restore(const QByteArray &bookmarkData,
                                                            const QString &filePath)
{
    std::unique_ptr<SQLiteFileAccess> access;
    if (!bookmarkData.isEmpty()) {
        access = fromBookmark(bookmarkData);
    }
    if (!access) {
        access = fromFilePath(filePath);
    }
    return access;
}
