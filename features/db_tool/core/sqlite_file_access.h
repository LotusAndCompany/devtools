#ifndef SQLITE_FILE_ACCESS_H
#define SQLITE_FILE_ACCESS_H

#include <QByteArray>
#include <QString>

#include <memory>

class SQLiteFileAccess final
{
public:
    static std::unique_ptr<SQLiteFileAccess> fromFilePath(const QString &filePath);
    static std::unique_ptr<SQLiteFileAccess> fromBookmark(const QByteArray &bookmarkData);

    ~SQLiteFileAccess();
    SQLiteFileAccess(const SQLiteFileAccess &) = delete;
    SQLiteFileAccess &operator=(const SQLiteFileAccess &) = delete;
    SQLiteFileAccess(SQLiteFileAccess &&) = delete;
    SQLiteFileAccess &operator=(SQLiteFileAccess &&) = delete;

    [[nodiscard]] const QString &filePath() const;
    [[nodiscard]] const QByteArray &bookmarkData() const;

private:
    struct Impl;

    explicit SQLiteFileAccess(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> impl;
};

#endif // SQLITE_FILE_ACCESS_H
