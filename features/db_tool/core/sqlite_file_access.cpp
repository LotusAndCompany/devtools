#include "sqlite_file_access.h"

#include <utility>

struct SQLiteFileAccess::Impl
{
    QString filePath;
    QByteArray bookmarkData;
};

SQLiteFileAccess::SQLiteFileAccess(std::unique_ptr<Impl> impl) : impl(std::move(impl)) {}

SQLiteFileAccess::~SQLiteFileAccess() = default;

std::unique_ptr<SQLiteFileAccess> SQLiteFileAccess::fromFilePath(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return nullptr;
    }

    auto impl = std::make_unique<SQLiteFileAccess::Impl>();
    impl->filePath = filePath;
    return std::unique_ptr<SQLiteFileAccess>(new SQLiteFileAccess(std::move(impl)));
}

std::unique_ptr<SQLiteFileAccess> SQLiteFileAccess::fromBookmark(const QByteArray &bookmarkData)
{
    Q_UNUSED(bookmarkData);
    return nullptr;
}

const QString &SQLiteFileAccess::filePath() const
{
    return impl->filePath;
}

const QByteArray &SQLiteFileAccess::bookmarkData() const
{
    return impl->bookmarkData;
}
