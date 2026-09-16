#include "timestamp_conversion.h"

#include <QTimeZone>

QDateTime TimestampConversion::fromUnixTimestamp(const QString &text, Unit unit, bool *ok)
{
    bool parsedOk = false;
    const qint64 value = text.trimmed().toLongLong(&parsedOk);

    const QDateTime utcInstant =
        parsedOk ? (unit == Unit::Milliseconds
                        ? QDateTime::fromMSecsSinceEpoch(value, QTimeZone::UTC)
                        : QDateTime::fromSecsSinceEpoch(value, QTimeZone::UTC))
                 : QDateTime();

    const bool success = parsedOk && utcInstant.isValid();
    if (ok != nullptr) {
        *ok = success;
    }
    return success ? utcInstant : QDateTime();
}

QDateTime TimestampConversion::fromIso8601(const QString &text, bool *ok)
{
    const QString trimmed = text.trimmed();
    QDateTime parsed = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
    if (!parsed.isValid()) {
        // ミリ秒を含まないISO8601形式も許容する
        parsed = QDateTime::fromString(trimmed, Qt::ISODate);
    }

    if (ok != nullptr) {
        *ok = parsed.isValid();
    }
    return parsed.isValid() ? parsed.toUTC() : QDateTime();
}

QDateTime TimestampConversion::fromLocalDateTime(const QDateTime &localDateTime)
{
    if (!localDateTime.isValid()) {
        return {};
    }

    const QDateTime interpreted(localDateTime.date(), localDateTime.time(),
                                 QTimeZone::systemTimeZone());
    return interpreted.toUTC();
}

QDateTime TimestampConversion::fromUtcDateTime(const QDateTime &utcDateTime)
{
    if (!utcDateTime.isValid()) {
        return {};
    }

    return {utcDateTime.date(), utcDateTime.time(), QTimeZone::UTC};
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

    // Qt::LocalTimeのtimeSpecはtoString(Qt::ISODateWithMs)でオフセットを付与しないため、
    // 明示的にQTimeZoneへ変換してオフセット付きの文字列を得る
    const QDateTime local = utcInstant.toLocalTime();
    const QDateTime localWithZone(local.date(), local.time(), QTimeZone::systemTimeZone());
    return localWithZone.toString(Qt::ISODateWithMs);
}

QDateTime TimestampConversion::toLocalDateTime(const QDateTime &utcInstant)
{
    if (!utcInstant.isValid()) {
        return {};
    }

    return utcInstant.toLocalTime();
}
