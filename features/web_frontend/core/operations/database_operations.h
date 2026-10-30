#ifndef FRONTEND_DATABASE_OPERATIONS_H
#define FRONTEND_DATABASE_OPERATIONS_H

#include "frontend_operation_service.h"

namespace FrontendOperationHandlers {
FrontendOperationService::Result databaseConnect(const QVariantMap &payload);
FrontendOperationService::Result databaseTables(const QVariantMap &payload);
FrontendOperationService::Result databaseQuery(const QVariantMap &payload);
} // namespace FrontendOperationHandlers

#endif // FRONTEND_DATABASE_OPERATIONS_H
