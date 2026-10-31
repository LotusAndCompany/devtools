#ifndef FRONTEND_HTTP_OPERATIONS_H
#define FRONTEND_HTTP_OPERATIONS_H

#include "frontend_operation_service.h"

namespace FrontendOperationHandlers {
FrontendOperationService::Result sendHttpRequest(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress);
} // namespace FrontendOperationHandlers

#endif // FRONTEND_HTTP_OPERATIONS_H
