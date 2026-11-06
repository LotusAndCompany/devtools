#ifndef FRONTEND_MARKDOWN_OPERATIONS_H
#define FRONTEND_MARKDOWN_OPERATIONS_H

#include "features/web_frontend/gui/bridge/operations/frontend_operation_service.h"

namespace FrontendOperationHandlers {
FrontendOperationService::Result renderMarkdown(const QVariantMap &payload);
} // namespace FrontendOperationHandlers

#endif // FRONTEND_MARKDOWN_OPERATIONS_H
