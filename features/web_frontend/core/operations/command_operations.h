#ifndef FRONTEND_COMMAND_OPERATIONS_H
#define FRONTEND_COMMAND_OPERATIONS_H

#include "frontend_operation_service.h"

namespace FrontendOperationHandlers {
FrontendOperationService::Result commandData(const QVariantMap &payload);
} // namespace FrontendOperationHandlers

#endif // FRONTEND_COMMAND_OPERATIONS_H
