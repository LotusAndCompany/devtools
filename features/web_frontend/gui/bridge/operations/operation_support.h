#ifndef FRONTEND_OPERATION_SUPPORT_H
#define FRONTEND_OPERATION_SUPPORT_H

#include "features/web_frontend/gui/bridge/bridge_limits.h"
#include "frontend_operation_service.h"

#include <QMetaType>

namespace FrontendOperationSupport {
inline FrontendOperationService::Result invalidInput(const QString &message)
{
    return {{}, QStringLiteral("INVALID_INPUT"), message, false};
}

inline bool readText(const QVariantMap &payload, const QString &key, QString *value)
{
    const QVariant field = payload.value(key);
    if (field.metaType().id() != QMetaType::QString) {
        return false;
    }
    *value = field.toString();
    return value->toUtf8().size() <= BridgeLimits::MAX_INPUT_BYTES;
}
} // namespace FrontendOperationSupport

#endif // FRONTEND_OPERATION_SUPPORT_H
