#ifndef FRONTEND_PHRASE_OPERATIONS_H
#define FRONTEND_PHRASE_OPERATIONS_H

#include "frontend_operation_service.h"

namespace FrontendOperationHandlers {
FrontendOperationService::Result phraseData(const QVariantMap &payload);
} // namespace FrontendOperationHandlers

#endif // FRONTEND_PHRASE_OPERATIONS_H
