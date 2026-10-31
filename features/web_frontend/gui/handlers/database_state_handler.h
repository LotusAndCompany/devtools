#ifndef DATABASE_STATE_HANDLER_H
#define DATABASE_STATE_HANDLER_H

#include <QByteArray>
#include <QVariantMap>

#include <memory>

class SQLiteFileAccess;

class DatabaseStateHandler
{
public:
    [[nodiscard]] bool connected() const;
    [[nodiscard]] const QVariantMap &connection() const;
    [[nodiscard]] QString selectedFilePath() const;
    [[nodiscard]] QString selectedFileName() const;
    [[nodiscard]] std::shared_ptr<SQLiteFileAccess> fileAccess() const;
    [[nodiscard]] bool selectFile(const QString &path);
    [[nodiscard]] bool restoreFileAccess(const QByteArray &bookmark, const QString &fallbackPath);
    void clearConnection();
    void disconnect();
    void clearConnectionForDroppedFile();
    void completeConnection(const QVariantMap &connection, const QString &typeText);

private:
    QVariantMap connection_;
    QString selected_file_path_;
    std::shared_ptr<SQLiteFileAccess> file_access_;
};

#endif // DATABASE_STATE_HANDLER_H
