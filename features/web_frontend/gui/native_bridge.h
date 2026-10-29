#ifndef NATIVE_BRIDGE_H
#define NATIVE_BRIDGE_H

#include <QFutureWatcher>
#include <QHash>
#include <QImage>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include <atomic>
#include <functional>
#include <memory>

class QFileDialog;
class SQLiteFileAccess;
class QWidget;

/**
 * @brief Versioned, bounded WebChannel API for the React frontend.
 */
class NativeBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString contractVersion MEMBER contract_version_ CONSTANT)

public:
    static constexpr qsizetype MAX_INPUT_BYTES = 1024 * 1024;
    static constexpr qsizetype MAX_OUTPUT_BYTES = 4 * 1024 * 1024;
    static constexpr qsizetype MAX_MESSAGE_BYTES = 2 * 1024 * 1024;
    static constexpr int MAX_CONCURRENT_OPERATIONS = 4;

    explicit NativeBridge(QWidget *dialogParent, QObject *parent = nullptr);
    ~NativeBridge() override;

    Q_INVOKABLE void startOperation(const QVariantMap &request);
    Q_INVOKABLE void cancelOperation(const QString &requestId);
    Q_INVOKABLE void navigateTo(const QString &route);
    void openDroppedSQLiteFile(const QString &path);
    void openDroppedImageFile(const QString &path);

Q_SIGNALS:
    void operationStarted(const QString &requestId, const QString &operation);
    void operationProgress(const QString &requestId, int percent, const QString &message);
    void operationCompleted(const QString &requestId, const QVariantMap &result);
    void operationFailed(const QString &requestId, const QVariantMap &error);
    void operationCancelled(const QString &requestId);
    void navigationRequested(const QString &route);
    void languageChangeRequested(const QString &languageCode);
    void imageDropAvailable();

private:
    friend class TestNativeBridge;

    struct OperationResult
    {
        QVariantMap value;
        QString errorCode;
        QString errorMessage;
        bool cancelled = false;
    };

    using CancellationToken = std::shared_ptr<std::atomic_bool>;
    using ProgressCallback = std::function<void(int, const QString &)>;
    using Worker =
        std::function<OperationResult(const CancellationToken &, const ProgressCallback &)>;

    struct RunningOperation
    {
        QString operation;
        CancellationToken cancellation;
        QFutureWatcher<OperationResult> *watcher = nullptr;
        QFileDialog *dialog = nullptr;
    };

    QWidget *dialog_parent_;
    QHash<QString, RunningOperation> operations_;
    QString contract_version_ = QStringLiteral("2");
    QImage original_image_;
    QImage current_image_;
    QString current_image_path_;
    QString dropped_image_path_;
    QVariantMap database_connection_;
    QString selected_sqlite_path_;
    std::shared_ptr<SQLiteFileAccess> sqlite_file_access_;

    [[nodiscard]] bool validateRequest(const QString &requestId, const QVariantMap &request);
    [[nodiscard]] bool textPayload(const QString &requestId, const QVariantMap &payload,
                                   const QString &sizeError, QString *text);
    [[nodiscard]] bool parseIndentation(const QString &requestId, const QVariantMap &payload,
                                        int *indentation);
    void startJsonFormat(const QString &requestId, const QVariantMap &payload);
    void startToolOperation(const QString &requestId, const QString &operation,
                            const QVariantMap &payload);
    void startImageOperation(const QString &requestId, const QString &operation,
                             const QVariantMap &payload);
    void startQrImageSaveOperation(const QString &requestId, const QVariantMap &payload);
    void startDatabaseOperation(const QString &requestId, const QString &operation,
                                const QVariantMap &payload);
    void completeOnUiThread(const QString &requestId, const QVariantMap &result);
    void startSaveSettings(const QString &requestId, const QVariantMap &payload);
    void fail(const QString &requestId, const QString &code, const QString &message);
    void startWorker(const QString &requestId, const QString &operation, Worker worker);
    void reportProgress(const QString &requestId, int percent, const QString &message);
    void startFileDialog(const QString &requestId, const QString &operation,
                         const QVariantMap &payload);
    [[nodiscard]] static OperationResult formatJson(const QString &text, int indentation,
                                                    const CancellationToken &cancellation,
                                                    const ProgressCallback &progress);
    [[nodiscard]] static OperationResult readTextFile(const QString &path,
                                                      const CancellationToken &cancellation,
                                                      const ProgressCallback &progress);
    [[nodiscard]] static OperationResult saveTextFile(const QString &path, const QString &text,
                                                      const CancellationToken &cancellation,
                                                      const ProgressCallback &progress);
    [[nodiscard]] static OperationResult readImageFile(const QString &path,
                                                       const CancellationToken &cancellation,
                                                       const ProgressCallback &progress);
    [[nodiscard]] static bool isValidRequestId(const QString &requestId);
    [[nodiscard]] static QString safeSuggestedName(const QString &name);
};

#endif // NATIVE_BRIDGE_H
