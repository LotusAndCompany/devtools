#ifndef FRONTEND_DATA_OPERATIONS_H
#define FRONTEND_DATA_OPERATIONS_H

#include "frontend_operation_service.h"

namespace FrontendOperationHandlers {
FrontendOperationService::Result convertData(const QVariantMap &payload);
FrontendOperationService::Result convertTimestamp(const QVariantMap &payload);
} // namespace FrontendOperationHandlers

#endif // FRONTEND_DATA_OPERATIONS_H
