#ifndef FRONTEND_OPERATION_SERVICE_H
#define FRONTEND_OPERATION_SERVICE_H

#include <QVariantMap>

#include <atomic>
#include <functional>
#include <memory>

struct FrontendOperationResult
{
    QVariantMap value;
    QString errorCode;
    QString errorMessage;
    bool cancelled{false};
};

class FrontendOperationService
{
public:
    using Result = FrontendOperationResult;
    using CancellationToken = std::shared_ptr<std::atomic_bool>;
    using ProgressCallback = std::function<void(int, const QString &)>;

    static Result execute(const QString &operation, const QVariantMap &payload,
                          const CancellationToken &cancellation, const ProgressCallback &progress);
};

#endif // FRONTEND_OPERATION_SERVICE_H
