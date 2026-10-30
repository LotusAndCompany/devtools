#ifndef FRONTEND_QR_OPERATIONS_H
#define FRONTEND_QR_OPERATIONS_H

#include "frontend_operation_service.h"

namespace FrontendOperationHandlers {
FrontendOperationService::Result generateQr(const QVariantMap &payload);
} // namespace FrontendOperationHandlers

#endif // FRONTEND_QR_OPERATIONS_H
