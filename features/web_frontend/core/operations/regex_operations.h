#ifndef FRONTEND_REGEX_OPERATIONS_H
#define FRONTEND_REGEX_OPERATIONS_H

#include "frontend_operation_service.h"

namespace FrontendOperationHandlers {
FrontendOperationService::Result evaluateRegex(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress);
} // namespace FrontendOperationHandlers

#endif // FRONTEND_REGEX_OPERATIONS_H
