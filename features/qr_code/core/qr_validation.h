#ifndef QR_VALIDATION_H
#define QR_VALIDATION_H

#include <QList>
#include <QString>
#include <QVariantMap>

#include <cstdint>

enum class QRCodeType : uint8_t { Text, Url, Email, Phone, Sms, Wifi, Contact, Calendar, Geo };

enum class QRValidationCode : uint8_t {
    TextEmpty,
    UrlEmpty,
    UrlScheme,
    EmailEmpty,
    EmailInvalid,
    PhoneEmpty,
    PhoneInvalid,
    MessageEmpty,
    SsidEmpty,
    PasswordEmpty,
    NameEmpty,
    ContactEmpty,
    SummaryEmpty,
    StartMissing,
    EndMissing,
    EndBeforeStart,
    LatitudeEmpty,
    LatitudeInvalid,
    LongitudeEmpty,
    LongitudeInvalid
};

struct QRValidationError
{
    QString field;
    QRValidationCode code;
};

class QRValidation
{
public:
    static QList<QRValidationError> validate(QRCodeType type, const QVariantMap &parameters);
    static bool canGenerate(QRCodeType type, const QVariantMap &parameters);
    static bool isValidPhoneNumber(const QString &phone);
};

#endif // QR_VALIDATION_H
