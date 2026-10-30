#include "features/web_frontend/gui/bridge/operation_registry.h"
#include "features/web_frontend/gui/native_bridge.h"

#include <QJsonDocument>
#include <QMetaType>
#include <QRegularExpression>

bool NativeBridge::validateRequest(const QString &requestId, const QVariantMap &request)
{
    if (!isValidRequestId(requestId)) {
        fail(QString(), QStringLiteral("INVALID_REQUEST"),
             QStringLiteral("Request ID must be 1 to 80 ASCII letters, digits, '_' or '-'."));
        return false;
    }
    if (request.value(QStringLiteral("contractVersion")).toString() != contract_version_) {
        fail(requestId, QStringLiteral("UNSUPPORTED_VERSION"),
             QStringLiteral("The bridge contract version is not supported."));
        return false;
    }
    const QVariant payloadValue = request.value(QStringLiteral("payload"));
    if (payloadValue.metaType().id() != QMetaType::QVariantMap) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("The request payload must be an object."));
        return false;
    }
    const QVariantMap payload = payloadValue.toMap();
    if (QJsonDocument::fromVariant(payload).toJson(QJsonDocument::Compact).size() >
        MAX_MESSAGE_BYTES) {
        fail(requestId, QStringLiteral("SIZE_LIMIT_EXCEEDED"),
             QStringLiteral("The request exceeds the 2 MiB message limit."));
        return false;
    }
    if (operations_.contains(requestId)) {
        fail(requestId, QStringLiteral("DUPLICATE_REQUEST"),
             QStringLiteral("A request with this ID is already running."));
        return false;
    }
    if (operations_.size() >= MAX_CONCURRENT_OPERATIONS) {
        fail(requestId, QStringLiteral("TOO_MANY_REQUESTS"),
             QStringLiteral("The maximum number of concurrent operations was reached."));
        return false;
    }

    const QString operation = request.value(QStringLiteral("operation")).toString();
    if (!operationHandler(operation).has_value()) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("The requested operation is not available."));
        return false;
    }
    return true;
}

bool NativeBridge::textPayload(const QString &requestId, const QVariantMap &payload,
                               const QString &sizeError, QString *text)
{
    const QVariant textValue = payload.value(QStringLiteral("text"));
    if (textValue.metaType().id() != QMetaType::QString) {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("The text field must be a string."));
        return false;
    }
    *text = textValue.toString();
    if (text->toUtf8().size() > MAX_INPUT_BYTES) {
        fail(requestId, QStringLiteral("SIZE_LIMIT_EXCEEDED"), sizeError);
        return false;
    }
    return true;
}

bool NativeBridge::parseIndentation(const QString &requestId, const QVariantMap &payload,
                                    int *indentation)
{
    const QString value = payload.value(QStringLiteral("indentation")).toString();
    if (value == QStringLiteral("2")) {
        *indentation = 0;
    } else if (value == QStringLiteral("4")) {
        *indentation = 1;
    } else if (value == QStringLiteral("tab")) {
        *indentation = 2;
    } else if (value == QStringLiteral("minified")) {
        *indentation = 3;
    } else {
        fail(requestId, QStringLiteral("INVALID_INPUT"),
             QStringLiteral("Indentation must be 2, 4, tab or minified."));
        return false;
    }
    return true;
}

bool NativeBridge::isValidRequestId(const QString &requestId)
{
    static const QRegularExpression pattern(QStringLiteral("^[A-Za-z0-9_-]{1,80}$"));
    return pattern.match(requestId).hasMatch();
}
