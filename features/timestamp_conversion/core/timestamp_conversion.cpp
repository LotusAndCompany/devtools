#include "timestamp_conversion.h"

#include <QTimeZone>

QDateTime TimestampConversion::fromUnixTimestamp(const QString &text, Unit unit)
{
    bool parsedOk = false;
    const qint64 value = text.trimmed().toLongLong(&parsedOk);
    if (!parsedOk) {
        return {};
    }

    return (unit == Unit::Milliseconds) ? QDateTime::fromMSecsSinceEpoch(value, QTimeZone::UTC)
                                        : QDateTime::fromSecsSinceEpoch(value, QTimeZone::UTC);
}

QDateTime TimestampConversion::fromIso8601(const QString &text)
{
    const QDateTime parsed = QDateTime::fromString(text.trimmed(), Qt::ISODateWithMs);
    return parsed.isValid() ? parsed.toUTC() : QDateTime();
}

QString TimestampConversion::toUnixTimestamp(const QDateTime &utcInstant, Unit unit)
{
    if (!utcInstant.isValid()) {
        return {};
    }

    return (unit == Unit::Milliseconds) ? QString::number(utcInstant.toMSecsSinceEpoch())
                                        : QString::number(utcInstant.toSecsSinceEpoch());
}

QString TimestampConversion::toIso8601(const QDateTime &utcInstant)
{
    if (!utcInstant.isValid()) {
        return {};
    }

    return utcInstant.toUTC().toString(Qt::ISODateWithMs);
}

QString TimestampConversion::toIso8601Local(const QDateTime &utcInstant)
{
    if (!utcInstant.isValid()) {
        return {};
    }

    return utcInstant.toTimeZone(QTimeZone::systemTimeZone()).toString(Qt::ISODateWithMs);
}
