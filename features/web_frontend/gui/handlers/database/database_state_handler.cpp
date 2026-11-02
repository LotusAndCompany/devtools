#include "database_state_handler.h"

#include "features/db_tool/core/connection_history.h"
#include "features/db_tool/core/database_connection.h"
#include "features/db_tool/core/sqlite_file_access.h"

#include <QFileInfo>
#include <QSettings>

bool DatabaseStateHandler::connected() const
{
    return !connection_.isEmpty();
}

const QVariantMap &DatabaseStateHandler::connection() const
{
    return connection_;
}

QString DatabaseStateHandler::selectedFilePath() const
{
    return selected_file_path_;
}

QString DatabaseStateHandler::selectedFileName() const
{
    return QFileInfo(selected_file_path_).fileName();
}

std::shared_ptr<SQLiteFileAccess> DatabaseStateHandler::fileAccess() const
{
    return file_access_;
}

bool DatabaseStateHandler::selectFile(const QString &path)
{
    std::unique_ptr<SQLiteFileAccess> access = SQLiteFileAccess::fromFilePath(path);
    if (!access) {
        return false;
    }
    file_access_ = std::shared_ptr<SQLiteFileAccess>(std::move(access));
    selected_file_path_ = file_access_->filePath();
    return true;
}

bool DatabaseStateHandler::restoreFileAccess(const QByteArray &bookmark,
                                             const QString &fallbackPath)
{
    std::unique_ptr<SQLiteFileAccess> access =
        bookmark.isEmpty() ? nullptr : SQLiteFileAccess::fromBookmark(bookmark);
    if (!access) {
        access = SQLiteFileAccess::fromFilePath(fallbackPath);
    }
    if (!access) {
        return false;
    }
    file_access_ = std::shared_ptr<SQLiteFileAccess>(std::move(access));
    selected_file_path_ = file_access_->filePath();
    return true;
}

void DatabaseStateHandler::clearConnection()
{
    connection_.clear();
}

void DatabaseStateHandler::disconnect()
{
    connection_.clear();
    file_access_.reset();
    selected_file_path_.clear();
}

void DatabaseStateHandler::clearConnectionForDroppedFile()
{
    connection_.clear();
}

void DatabaseStateHandler::completeConnection(const QVariantMap &connection,
                                              const QString &typeText)
{
    connection_ = connection;
    const DatabaseParameters parameters{connection.value(QStringLiteral("type")).toString(),
                                        connection.value(QStringLiteral("host")).toString(),
                                        connection.value(QStringLiteral("database")).toString(),
                                        connection.value(QStringLiteral("username")).toString(),
                                        connection.value(QStringLiteral("password")).toString()};
    const QByteArray bookmark = file_access_ ? file_access_->bookmarkData() : QByteArray{};
    const QJsonObject historyEntry =
        DatabaseConnection::connectionInfo(parameters, typeText, bookmark);
    QSettings settings;
    ConnectionHistory::add(settings, historyEntry);
}
