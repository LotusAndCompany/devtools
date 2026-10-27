#include "qr_validation.h"

#include <QDateTime>
#include <QRegularExpression>

namespace {
void validatePhone(QList<QRValidationError> &errors, const QString &field, const QString &phone)
{
    if (phone.trimmed().isEmpty()) {
        errors.append({field, QRValidationCode::PhoneEmpty});
    } else if (!QRValidation::isValidPhoneNumber(phone)) {
        errors.append({field, QRValidationCode::PhoneInvalid});
    }
}

void validateCoordinate(QList<QRValidationError> &errors, const QString &field,
                        const QString &value, double limit, QRValidationCode empty,
                        QRValidationCode invalid)
{
    if (value.trimmed().isEmpty()) {
        errors.append({field, empty});
        return;
    }
    bool ok;
    const double number = value.toDouble(&ok);
    if (!ok || number < -limit || number > limit) {
        errors.append({field, invalid});
    }
}

void validateContact(QList<QRValidationError> &errors, const QVariantMap &parameters)
{
    if (parameters.value("contact_name").toString().trimmed().isEmpty()) {
        errors.append({"contact_name_error", QRValidationCode::NameEmpty});
    }
    const QString phone = parameters.value("contact_phone").toString();
    const QString email = parameters.value("contact_email").toString();
    const bool has_phone = !phone.trimmed().isEmpty();
    const bool has_email = !email.trimmed().isEmpty();
    if (!has_phone && !has_email) {
        errors.append({"contact_phone_error", QRValidationCode::ContactEmpty});
    }
    if (has_phone && !QRValidation::isValidPhoneNumber(phone)) {
        errors.append({"contact_phone_error", QRValidationCode::PhoneInvalid});
    }
    if (has_email && !email.contains('@')) {
        errors.append({"contact_email_error", QRValidationCode::EmailInvalid});
    }
}

void validateCalendar(QList<QRValidationError> &errors, const QVariantMap &parameters)
{
    if (parameters.value("cal_summary").toString().trimmed().isEmpty()) {
        errors.append({"cal_summary_error", QRValidationCode::SummaryEmpty});
    }
    if (!parameters.contains("cal_start")) {
        errors.append({"cal_start_error", QRValidationCode::StartMissing});
    }
    if (!parameters.contains("cal_end")) {
        errors.append({"cal_end_error", QRValidationCode::EndMissing});
    }
    if (parameters.contains("cal_start") && parameters.contains("cal_end") &&
        parameters.value("cal_start").toDateTime() >= parameters.value("cal_end").toDateTime()) {
        errors.append({"cal_end_error", QRValidationCode::EndBeforeStart});
    }
}

void validateWifi(QList<QRValidationError> &errors, const QVariantMap &parameters)
{
    if (parameters.value("wifi_ssid").toString().trimmed().isEmpty()) {
        errors.append({"wifi_ssid_error", QRValidationCode::SsidEmpty});
    }
    const QString security = parameters.value("wifi_security").toString();
    if ((security == "WPA" || security == "WEP") &&
        parameters.value("wifi_password").toString().trimmed().isEmpty()) {
        errors.append({"wifi_password_error", QRValidationCode::PasswordEmpty});
    }
}
} // namespace

bool QRValidation::isValidPhoneNumber(const QString &phone)
{
    if (phone.trimmed().isEmpty()) {
        return false;
    }
    QString const cleanPhone = phone.trimmed();
    QRegularExpression const internationalPattern(
        R"(^\+\d{1,3}[-\s]?\d{1,4}[-\s]?\d{4}[-\s]?\d{4}$)");
    if (internationalPattern.match(cleanPhone).hasMatch()) {
        return true;
    }
    QRegularExpression const domesticPattern(R"(^0\d{1,4}[-\s]?\d{2,4}[-\s]?\d{4}$)");
    if (domesticPattern.match(cleanPhone).hasMatch()) {
        return true;
    }
    QRegularExpression const digitOnlyPattern(R"(^\d{10,11}$)");
    return digitOnlyPattern.match(cleanPhone).hasMatch();
}

QList<QRValidationError> QRValidation::validate(QRCodeType type, const QVariantMap &parameters)
{
    QList<QRValidationError> errors;
    const auto value = [&parameters](const char *key) {
        return parameters.value(key).toString();
    };
    switch (type) {
    case QRCodeType::Text:
        if (value("text_content").trimmed().isEmpty()) {
            errors.append({"text_content_error", QRValidationCode::TextEmpty});
        }
        break;
    case QRCodeType::Url: {
        const QString url = value("url_content");
        if (url.trimmed().isEmpty()) {
            errors.append({"url_content_error", QRValidationCode::UrlEmpty});
        } else if (!url.startsWith("http://") && !url.startsWith("https://")) {
            errors.append({"url_content_error", QRValidationCode::UrlScheme});
        }
        break;
    }
    case QRCodeType::Email: {
        const QString email = value("email_address");
        if (email.trimmed().isEmpty()) {
            errors.append({"email_address_error", QRValidationCode::EmailEmpty});
        } else if (!email.contains('@')) {
            errors.append({"email_address_error", QRValidationCode::EmailInvalid});
        }
        break;
    }
    case QRCodeType::Phone:
        validatePhone(errors, "phone_number_error", value("phone_number"));
        break;
    case QRCodeType::Sms:
        validatePhone(errors, "sms_phone_error", value("sms_phone"));
        if (value("sms_message").trimmed().isEmpty()) {
            errors.append({"sms_message_error", QRValidationCode::MessageEmpty});
        }
        break;
    case QRCodeType::Wifi:
        validateWifi(errors, parameters);
        break;
    case QRCodeType::Contact: {
        validateContact(errors, parameters);
        break;
    }
    case QRCodeType::Calendar: {
        validateCalendar(errors, parameters);
        break;
    }
    case QRCodeType::Geo:
        validateCoordinate(errors, "geo_lat_error", value("geo_lat"), 90.0,
                           QRValidationCode::LatitudeEmpty, QRValidationCode::LatitudeInvalid);
        validateCoordinate(errors, "geo_lng_error", value("geo_lng"), 180.0,
                           QRValidationCode::LongitudeEmpty, QRValidationCode::LongitudeInvalid);
        break;
    }
    return errors;
}

bool QRValidation::canGenerate(QRCodeType type, const QVariantMap &parameters)
{
    const auto filled = [&parameters](const char *key) {
        return !parameters.value(key).toString().trimmed().isEmpty();
    };
    switch (type) {
    case QRCodeType::Text:
        return filled("text_content");
    case QRCodeType::Url:
        return filled("url_content");
    case QRCodeType::Email:
        return filled("email_address");
    case QRCodeType::Phone:
        return filled("phone_number");
    case QRCodeType::Sms:
        return filled("sms_phone") && filled("sms_message");
    case QRCodeType::Wifi: {
        if (!parameters.contains("wifi_security")) {
            return false;
        }
        const QString security = parameters.value("wifi_security").toString();
        return filled("wifi_ssid") &&
               (!(security == "WPA" || security == "WEP") ||
                !parameters.contains("wifi_password") || filled("wifi_password"));
    }
    case QRCodeType::Contact:
        return filled("contact_name") && (filled("contact_phone") || filled("contact_email"));
    case QRCodeType::Calendar:
        return filled("cal_summary") && parameters.contains("cal_start") &&
               parameters.contains("cal_end");
    case QRCodeType::Geo:
        return filled("geo_lat") && filled("geo_lng");
    }
    return false;
}
