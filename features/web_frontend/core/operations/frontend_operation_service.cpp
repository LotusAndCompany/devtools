#include "frontend_operation_service.h"

#include "command_operations.h"
#include "data_operations.h"
#include "database_operations.h"
#include "diff_operations.h"
#include "http_operations.h"
#include "markdown_operations.h"
#include "operation_support.h"
#include "phrase_operations.h"
#include "qr_operations.h"
#include "regex_operations.h"

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
