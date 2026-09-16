#include "features/timestamp_conversion/core/timestamp_conversion.h"

#include <QtTest>

namespace Test {
class TestTimestampConversion : public QObject
{
    Q_OBJECT

private slots:
    // Test cases:
    static void test_fromUnixTimestamp_seconds();
    static void test_fromUnixTimestamp_milliseconds();
    static void test_fromUnixTimestamp_invalidText();
    static void test_fromIso8601_withMilliseconds();
    static void test_fromIso8601_withoutMilliseconds();
    static void test_fromIso8601_invalidText();
    static void test_fromLocalDateTime();
    static void test_fromLocalDateTime_invalid();
    static void test_fromUtcDateTime();
    static void test_fromUtcDateTime_invalid();
    static void test_toUnixTimestamp_seconds();
    static void test_toUnixTimestamp_milliseconds();
    static void test_toUnixTimestamp_invalid();
    static void test_toIso8601();
    static void test_toIso8601_invalid();
    static void test_toIso8601Local_roundTrip();
    static void test_toIso8601Local_invalid();
    static void test_toLocalDateTime();
    static void test_roundTrip_allRepresentations();
};

void TestTimestampConversion::test_fromUnixTimestamp_seconds()
{
    bool ok = false;
    const QDateTime result = TimestampConversion::fromUnixTimestamp(
        "1700000000", TimestampConversion::Unit::Seconds, &ok);

    QVERIFY(ok);
    QCOMPARE(result, QDateTime::fromSecsSinceEpoch(1700000000, Qt::UTC));
}

void TestTimestampConversion::test_fromUnixTimestamp_milliseconds()
{
    bool ok = false;
    const QDateTime result = TimestampConversion::fromUnixTimestamp(
        "1700000000497", TimestampConversion::Unit::Milliseconds, &ok);

    QVERIFY(ok);
    QCOMPARE(result, QDateTime::fromMSecsSinceEpoch(1700000000497, Qt::UTC));
}

void TestTimestampConversion::test_fromUnixTimestamp_invalidText()
{
    bool ok = false;
    const QDateTime result = TimestampConversion::fromUnixTimestamp(
        "not a number", TimestampConversion::Unit::Seconds, &ok);

    QVERIFY(!ok);
    QVERIFY(!result.isValid());
}

void TestTimestampConversion::test_fromIso8601_withMilliseconds()
{
    bool ok = false;
    const QDateTime result = TimestampConversion::fromIso8601("2026-09-13T20:32:37.497Z", &ok);

    QVERIFY(ok);
    QCOMPARE(result,
              QDateTime::fromString("2026-09-13T20:32:37.497Z", Qt::ISODateWithMs).toUTC());
}

void TestTimestampConversion::test_fromIso8601_withoutMilliseconds()
{
    bool ok = false;
    const QDateTime result = TimestampConversion::fromIso8601("2023-11-14T22:13:20Z", &ok);

    QVERIFY(ok);
    QCOMPARE(result, QDateTime::fromSecsSinceEpoch(1700000000, Qt::UTC));
}

void TestTimestampConversion::test_fromIso8601_invalidText()
{
    bool ok = false;
    const QDateTime result = TimestampConversion::fromIso8601("not a date", &ok);

    QVERIFY(!ok);
    QVERIFY(!result.isValid());
}

void TestTimestampConversion::test_fromLocalDateTime()
{
    const QDateTime localDateTime(QDate(2023, 11, 14), QTime(22, 13, 20));
    const QDateTime result = TimestampConversion::fromLocalDateTime(localDateTime);

    const QDateTime expected(QDate(2023, 11, 14), QTime(22, 13, 20), Qt::LocalTime);
    QCOMPARE(result, expected.toUTC());
}

void TestTimestampConversion::test_fromLocalDateTime_invalid()
{
    const QDateTime result = TimestampConversion::fromLocalDateTime(QDateTime());
    QVERIFY(!result.isValid());
}

void TestTimestampConversion::test_fromUtcDateTime()
{
    const QDateTime utcWallClock(QDate(2023, 11, 14), QTime(22, 13, 20));
    const QDateTime result = TimestampConversion::fromUtcDateTime(utcWallClock);

    QCOMPARE(result, QDateTime(QDate(2023, 11, 14), QTime(22, 13, 20), Qt::UTC));
}

void TestTimestampConversion::test_fromUtcDateTime_invalid()
{
    const QDateTime result = TimestampConversion::fromUtcDateTime(QDateTime());
    QVERIFY(!result.isValid());
}

void TestTimestampConversion::test_toUnixTimestamp_seconds()
{
    const QDateTime utcInstant = QDateTime::fromSecsSinceEpoch(1700000000, Qt::UTC);
    const QString result =
        TimestampConversion::toUnixTimestamp(utcInstant, TimestampConversion::Unit::Seconds);

    QCOMPARE(result, QStringLiteral("1700000000"));
}

void TestTimestampConversion::test_toUnixTimestamp_milliseconds()
{
    const QDateTime utcInstant = QDateTime::fromMSecsSinceEpoch(1700000000497, Qt::UTC);
    const QString result =
        TimestampConversion::toUnixTimestamp(utcInstant, TimestampConversion::Unit::Milliseconds);

    QCOMPARE(result, QStringLiteral("1700000000497"));
}

void TestTimestampConversion::test_toUnixTimestamp_invalid()
{
    const QString result =
        TimestampConversion::toUnixTimestamp(QDateTime(), TimestampConversion::Unit::Seconds);
    QVERIFY(result.isEmpty());
}

void TestTimestampConversion::test_toIso8601()
{
    const QDateTime utcInstant = QDateTime::fromSecsSinceEpoch(1700000000, Qt::UTC);
    const QString result = TimestampConversion::toIso8601(utcInstant);

    QCOMPARE(result, QStringLiteral("2023-11-14T22:13:20.000Z"));
}

void TestTimestampConversion::test_toIso8601_invalid()
{
    const QString result = TimestampConversion::toIso8601(QDateTime());
    QVERIFY(result.isEmpty());
}

void TestTimestampConversion::test_toIso8601Local_roundTrip()
{
    const QDateTime utcInstant = QDateTime::fromSecsSinceEpoch(1700000000, Qt::UTC);
    const QString result = TimestampConversion::toIso8601Local(utcInstant);

    QVERIFY(!result.isEmpty());

    bool ok = false;
    const QDateTime parsedBack = TimestampConversion::fromIso8601(result, &ok);
    QVERIFY(ok);
    QCOMPARE(parsedBack, utcInstant);
}

void TestTimestampConversion::test_toIso8601Local_invalid()
{
    const QString result = TimestampConversion::toIso8601Local(QDateTime());
    QVERIFY(result.isEmpty());
}

void TestTimestampConversion::test_toLocalDateTime()
{
    const QDateTime utcInstant = QDateTime::fromSecsSinceEpoch(1700000000, Qt::UTC);
    const QDateTime result = TimestampConversion::toLocalDateTime(utcInstant);

    QCOMPARE(result, utcInstant.toLocalTime());
}

void TestTimestampConversion::test_roundTrip_allRepresentations()
{
    bool ok = false;
    const QDateTime original = TimestampConversion::fromUnixTimestamp(
        "1700000000", TimestampConversion::Unit::Seconds, &ok);
    QVERIFY(ok);

    const QString iso = TimestampConversion::toIso8601(original);
    const QDateTime fromIso = TimestampConversion::fromIso8601(iso, &ok);
    QVERIFY(ok);
    QCOMPARE(fromIso, original);

    const QDateTime local = TimestampConversion::toLocalDateTime(original);
    const QDateTime fromLocal = TimestampConversion::fromLocalDateTime(local);
    QCOMPARE(fromLocal, original);

    const QString seconds =
        TimestampConversion::toUnixTimestamp(fromLocal, TimestampConversion::Unit::Seconds);
    QCOMPARE(seconds, QStringLiteral("1700000000"));
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestTimestampConversion)

#include "test_timestamp_conversion.moc"
