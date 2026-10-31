#ifndef FRONTEND_DIFF_OPERATIONS_H
#define FRONTEND_DIFF_OPERATIONS_H

#include "frontend_operation_service.h"

namespace FrontendOperationHandlers {
FrontendOperationService::Result compareText(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress);
} // namespace FrontendOperationHandlers

#endif // FRONTEND_DIFF_OPERATIONS_H
