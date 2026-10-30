#include "features/web_frontend/gui/handlers/image_state_handler.h"
#include "features/web_frontend/gui/native_bridge.h"

#include <QFileDialog>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

void NativeBridge::completeOnUiThread(const QString &requestId, const QVariantMap &result)
{
    operations_.remove(requestId);
    emit operationProgress(requestId, 100, QStringLiteral("Complete"));
    emit operationCompleted(requestId, result);
}

void NativeBridge::cancelOperation(const QString &requestId)
{
    auto it = operations_.find(requestId);
    if (it == operations_.end()) {
        return;
    }

    it->cancellation->store(true, std::memory_order_relaxed);
    if (it->dialog != nullptr) {
        it->dialog->reject();
    } else {
        reportProgress(requestId, 0, QStringLiteral("Cancellation requested"));
    }
}

void NativeBridge::fail(const QString &requestId, const QString &code, const QString &message)
{
    emit operationFailed(requestId,
                         {{QStringLiteral("code"), code}, {QStringLiteral("message"), message}});
}

void NativeBridge::startWorker(const QString &requestId, const QString &operation, Worker worker,
                               std::function<void(QVariantMap &)> onSuccess)
{
    auto it = operations_.find(requestId);
    if (it == operations_.end()) {
        return;
    }
    if (it->cancellation->load(std::memory_order_relaxed)) {
        operations_.erase(it);
        emit operationCancelled(requestId);
        return;
    }

    const CancellationToken cancellation = it->cancellation;
    const ProgressCallback progress = [this, requestId](int percent, const QString &message) {
        emit operationProgress(requestId, percent, message);
    };
    auto *watcher = new QFutureWatcher<OperationResult>(this);
    it->operation = operation;
    it->watcher = watcher;

    connect(watcher, &QFutureWatcher<OperationResult>::finished, this,
            [this, requestId, watcher, onSuccess = std::move(onSuccess)]() {
                const auto it = operations_.find(requestId);
                if (it == operations_.end() || it->watcher != watcher) {
                    watcher->deleteLater();
                    return;
                }

                const OperationResult result = watcher->result();
                operations_.erase(it);
                watcher->deleteLater();

                if (result.cancelled) {
                    emit operationCancelled(requestId);
                } else if (!result.errorCode.isEmpty()) {
                    fail(requestId, result.errorCode, result.errorMessage);
                } else {
                    QVariantMap publicResult = result.value;
                    if (onSuccess) {
                        onSuccess(publicResult);
                    }
                    emit operationProgress(requestId, 100, QStringLiteral("Complete"));
                    emit operationCompleted(requestId, publicResult);
                }
                if (image_state_->hasDroppedImage()) {
                    emit imageDropAvailable();
                }
            });

    watcher->setFuture(QtConcurrent::run([worker = std::move(worker), cancellation, progress]() {
        return worker(cancellation, progress);
    }));
}

void NativeBridge::reportProgress(const QString &requestId, int percent, const QString &message)
{
    emit operationProgress(requestId, percent, message);
}
