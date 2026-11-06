#include "frontend_operation_service.h"

#include "features/web_frontend/gui/bridge/operations/command/command_operations.h"
#include "features/web_frontend/gui/bridge/operations/data/data_operations.h"
#include "features/web_frontend/gui/bridge/operations/database/database_operations.h"
#include "features/web_frontend/gui/bridge/operations/diff/diff_operations.h"
#include "features/web_frontend/gui/bridge/operations/http/http_operations.h"
#include "features/web_frontend/gui/bridge/operations/markdown/markdown_operations.h"
#include "features/web_frontend/gui/bridge/operations/operation_support.h"
#include "features/web_frontend/gui/bridge/operations/phrase/phrase_operations.h"
#include "features/web_frontend/gui/bridge/operations/qr/qr_operations.h"
#include "features/web_frontend/gui/bridge/operations/regex/regex_operations.h"

using FrontendOperationSupport::invalidInput;

FrontendOperationService::Result FrontendOperationService::execute(
    const QString &operation, const QVariantMap &payload, const CancellationToken &cancellation,
    const ProgressCallback &progress)
{
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }
    if (operation == QStringLiteral("convertData")) {
        return FrontendOperationHandlers::convertData(payload);
    }
    if (operation == QStringLiteral("convertTimestamp")) {
        return FrontendOperationHandlers::convertTimestamp(payload);
    }
    if (operation == QStringLiteral("compareText")) {
        return FrontendOperationHandlers::compareText(payload, cancellation, progress);
    }
    if (operation == QStringLiteral("renderMarkdown")) {
        return FrontendOperationHandlers::renderMarkdown(payload);
    }
    if (operation == QStringLiteral("generateQr")) {
        return FrontendOperationHandlers::generateQr(payload);
    }
    if (operation == QStringLiteral("commandData")) {
        return FrontendOperationHandlers::commandData(payload);
    }
    if (operation == QStringLiteral("phraseData")) {
        return FrontendOperationHandlers::phraseData(payload);
    }
    if (operation == QStringLiteral("evaluateRegex")) {
        return FrontendOperationHandlers::evaluateRegex(payload, cancellation, progress);
    }
    if (operation == QStringLiteral("sendHttpRequest")) {
        return FrontendOperationHandlers::sendHttpRequest(payload, cancellation, progress);
    }
    if (operation == QStringLiteral("databaseConnect")) {
        return FrontendOperationHandlers::databaseConnect(payload);
    }
    if (operation == QStringLiteral("databaseTables")) {
        return FrontendOperationHandlers::databaseTables(payload);
    }
    if (operation == QStringLiteral("databaseQuery")) {
        return FrontendOperationHandlers::databaseQuery(payload);
    }
    return invalidInput(QStringLiteral("The requested tool operation is not available."));
}
