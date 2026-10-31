#include "native_bridge.h"

#include "features/web_frontend/gui/bridge/operation_registry.h"
#include "features/web_frontend/gui/handlers/database_state_handler.h"
#include "features/web_frontend/gui/handlers/image_state_handler.h"

#include <QFileDialog>

NativeBridge::NativeBridge(QWidget *dialogParent, QObject *parent)
    : QObject(parent)
    , dialog_parent_(dialogParent)
    , image_state_(std::make_unique<ImageStateHandler>())
    , database_state_(std::make_unique<DatabaseStateHandler>())
{}

NativeBridge::~NativeBridge()
{
    for (auto &operation : operations_) {
        operation.cancellation->store(true, std::memory_order_relaxed);
        if (operation.dialog != nullptr) {
            operation.dialog->disconnect(this);
            operation.dialog->reject();
        }
        if (operation.watcher != nullptr) {
            operation.watcher->future().waitForFinished();
        }
    }
}

void NativeBridge::startOperation(const QVariantMap &request)
{
    const QString requestId = request.value(QStringLiteral("requestId")).toString();
    if (!validateRequest(requestId, request)) {
        return;
    }

    const QString operation = request.value(QStringLiteral("operation")).toString();
    const QVariantMap payload = request.value(QStringLiteral("payload")).toMap();
    const auto cancellation = std::make_shared<std::atomic_bool>(false);
    operations_.insert(requestId, RunningOperation{operation, cancellation, nullptr, nullptr});
    emit operationStarted(requestId, operation);

    const auto handler = operationHandler(operation);
    Q_ASSERT(handler.has_value());
    switch (*handler) {
    case OperationHandler::JsonFormat:
        startJsonFormat(requestId, payload);
        return;
    case OperationHandler::Tool:
        startToolOperation(requestId, operation, payload);
        return;
    case OperationHandler::Image:
        startImageOperation(requestId, operation, payload);
        return;
    case OperationHandler::Database:
        startDatabaseOperation(requestId, operation, payload);
        return;
    case OperationHandler::QrImageSave:
        startQrImageSaveOperation(requestId, payload);
        return;
    case OperationHandler::RegexSettings:
        startRegexSettings(requestId, payload);
        return;
    case OperationHandler::SettingsRead:
        startSettingsRead(requestId);
        return;
    case OperationHandler::AboutInfo:
        startAboutInfo(requestId);
        return;
    case OperationHandler::SaveSettings:
        startSaveSettings(requestId, payload);
        return;
    case OperationHandler::SaveLastUsedTool:
        startSaveLastUsedTool(requestId, payload);
        return;
    case OperationHandler::Clipboard:
        startClipboard(requestId, payload);
        return;
    case OperationHandler::FileDialog:
        startFileDialog(requestId, operation, payload);
        return;
    }
}

void NativeBridge::navigateTo(const QString &route)
{
    static const QStringList allowedRoutes = {QStringLiteral("settings"), QStringLiteral("about"),
                                              QStringLiteral("welcome")};
    if (allowedRoutes.contains(route)) {
        emit navigationRequested(route);
    }
}
