#include "qr_operations.h"

#include "features/qr_code/core/content_generator.h"
#include "features/qr_code/core/qr_image.h"
#include "features/qr_code/core/qr_validation.h"
#include "operation_support.h"

#include <QBuffer>
#include <QDateTime>
#include <QIODevice>
#include <QMetaType>
#include <QVariantList>

using FrontendOperationSupport::invalidInput;

namespace FrontendOperationHandlers {

static QString qrImageData(const QImage &image)
{
    QImage preview = image;
    while (true) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        if (!buffer.open(QIODevice::WriteOnly) || !preview.save(&buffer, "PNG")) {
            return {};
        }
        if (bytes.size() <= 1024 * 1024) {
            return QStringLiteral("data:image/png;base64,") + QString::fromLatin1(bytes.toBase64());
        }
        if (preview.width() <= 128 || preview.height() <= 128) {
            return {};
        }
        preview = preview.scaled(preview.size() / 2, Qt::KeepAspectRatio, Qt::FastTransformation);
    }
}

FrontendOperationService::Result generateQr(const QVariantMap &payload)
{
    const QString typeName = payload.value(QStringLiteral("type")).toString();
    static const QStringList names = {
        QStringLiteral("text"),    QStringLiteral("url"),      QStringLiteral("email"),
        QStringLiteral("phone"),   QStringLiteral("sms"),      QStringLiteral("wifi"),
        QStringLiteral("contact"), QStringLiteral("calendar"), QStringLiteral("geo")};
    const int typeIndex = names.indexOf(typeName);
    if (typeIndex < 0) {
        return invalidInput(QStringLiteral("QR code type is not supported."));
    }
    const QVariantMap params = payload.value(QStringLiteral("parameters")).toMap();
    if (!QRValidation::canGenerate(static_cast<QRCodeType>(typeIndex), params)) {
        return invalidInput(QStringLiteral("Complete the required QR code fields."));
    }

    QVariantMap validatedParams = params;
    if (static_cast<QRCodeType>(typeIndex) == QRCodeType::Calendar) {
        for (const QString &key : {QStringLiteral("cal_start"), QStringLiteral("cal_end")}) {
            const QDateTime dateTime =
                QDateTime::fromString(params.value(key).toString(), Qt::ISODate);
            if (!dateTime.isValid()) {
                return invalidInput(QStringLiteral("Enter valid local calendar dates and times."));
            }
            validatedParams.insert(key, dateTime);
        }
    }
    const QList<QRValidationError> validationErrors =
        QRValidation::validate(static_cast<QRCodeType>(typeIndex), validatedParams);
    if (!validationErrors.isEmpty()) {
        return invalidInput(
            QStringLiteral("Invalid QR code field: %1.").arg(validationErrors.constFirst().field));
    }

    // QRValidation uses the current UI field names, while ContentGenerator
    // retains the legacy service keys. Keep that boundary conversion here.
    QVariantMap generatorParams;
    const auto copyField = [&params, &generatorParams](const char *source, const char *target) {
        const QString sourceKey = QString::fromLatin1(source);
        if (params.contains(sourceKey)) {
            generatorParams.insert(QString::fromLatin1(target), params.value(sourceKey));
        }
    };
    switch (static_cast<QRCodeType>(typeIndex)) {
    case QRCodeType::Text:
        copyField("text_content", "text");
        break;
    case QRCodeType::Url:
        copyField("url_content", "url");
        break;
    case QRCodeType::Email:
        copyField("email_address", "email");
        break;
    case QRCodeType::Phone:
        copyField("phone_number", "number");
        break;
    case QRCodeType::Sms:
        copyField("sms_phone", "number");
        copyField("sms_message", "text");
        break;
    case QRCodeType::Wifi:
        copyField("wifi_ssid", "ssid");
        copyField("wifi_password", "password");
        copyField("wifi_security", "type");
        copyField("wifi_hidden", "hidden");
        break;
    case QRCodeType::Contact:
        copyField("contact_name", "name");
        copyField("contact_phone", "phone");
        copyField("contact_email", "email");
        break;
    case QRCodeType::Calendar: {
        copyField("cal_summary", "summary");
        for (const auto &[source, target] :
             {std::pair{"cal_start", "start"}, std::pair{"cal_end", "end"}}) {
            const QDateTime dateTime = validatedParams.value(QLatin1String(source)).toDateTime();
            generatorParams.insert(QLatin1String(target), dateTime.toString("yyyyMMdd'T'hhmmss"));
        }
        break;
    }
    case QRCodeType::Geo:
        copyField("geo_lat", "lat");
        copyField("geo_lng", "lng");
        break;
    }

    const QString content = ContentGenerator::generate(typeName, generatorParams);
    if (content.isEmpty() || content.toUtf8().size() > 2953) {
        return invalidInput(QStringLiteral("QR code content is empty or exceeds its size limit."));
    }
    const QString image = qrImageData(QRImage::generate(content));
    if (image.isEmpty()) {
        return {{},
                QStringLiteral("INTERNAL_ERROR"),
                QStringLiteral("Could not create a QR preview."),
                false};
    }
    return {{{QStringLiteral("content"), content}, {QStringLiteral("imageDataUrl"), image}},
            {},
            {},
            false};
}

} // namespace FrontendOperationHandlers
