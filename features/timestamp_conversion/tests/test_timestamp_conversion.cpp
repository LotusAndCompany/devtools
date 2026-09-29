#include "features/timestamp_conversion/core/timestamp_conversion.h"

#include <QTimeZone>
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
    static void test_toUnixTimestamp_seconds();
    static void test_toUnixTimestamp_milliseconds();
    static void test_toUnixTimestamp_invalid();
    static void test_toIso8601();
    static void test_toIso8601_invalid();
    static void test_toIso8601Local_roundTrip();
    static void test_toIso8601Local_invalid();
    static void test_roundTrip_allRepresentations();
};

void TestTimestampConversion::test_fromUnixTimestamp_seconds()
{
    const QDateTime result =
        TimestampConversion::fromUnixTimestamp("1700000000", TimestampConversion::Unit::Seconds);

    QVERIFY(result.isValid());
    QCOMPARE(result, QDateTime::fromSecsSinceEpoch(1700000000, QTimeZone::UTC));
}

void TestTimestampConversion::test_fromUnixTimestamp_milliseconds()
{
    const QDateTime result = TimestampConversion::fromUnixTimestamp(
        "1700000000497", TimestampConversion::Unit::Milliseconds);

    QVERIFY(result.isValid());
    QCOMPARE(result, QDateTime::fromMSecsSinceEpoch(1700000000497, QTimeZone::UTC));
}

void TestTimestampConversion::test_fromUnixTimestamp_invalidText()
{
    const QDateTime result =
        TimestampConversion::fromUnixTimestamp("not a number", TimestampConversion::Unit::Seconds);

    QVERIFY(!result.isValid());
}

void TestTimestampConversion::test_fromIso8601_withMilliseconds()
{
    const QDateTime result = TimestampConversion::fromIso8601("2026-09-13T20:32:37.497Z");

    QVERIFY(result.isValid());
    QCOMPARE(result, QDateTime::fromString("2026-09-13T20:32:37.497Z", Qt::ISODateWithMs).toUTC());
}

void TestTimestampConversion::test_fromIso8601_withoutMilliseconds()
{
    const QDateTime result = TimestampConversion::fromIso8601("2023-11-14T22:13:20Z");

    QVERIFY(result.isValid());
    QCOMPARE(result, QDateTime::fromSecsSinceEpoch(1700000000, QTimeZone::UTC));
}

void TestTimestampConversion::test_fromIso8601_invalidText()
{
    const QDateTime result = TimestampConversion::fromIso8601("not a date");

    QVERIFY(!result.isValid());
}

void TestTimestampConversion::test_toUnixTimestamp_seconds()
{
    const QDateTime utcInstant = QDateTime::fromSecsSinceEpoch(1700000000, QTimeZone::UTC);
    const QString result =
        TimestampConversion::toUnixTimestamp(utcInstant, TimestampConversion::Unit::Seconds);

    QCOMPARE(result, QStringLiteral("1700000000"));
}

void TestTimestampConversion::test_toUnixTimestamp_milliseconds()
{
    const QDateTime utcInstant = QDateTime::fromMSecsSinceEpoch(1700000000497, QTimeZone::UTC);
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
    const QDateTime utcInstant = QDateTime::fromSecsSinceEpoch(1700000000, QTimeZone::UTC);
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
    const QDateTime utcInstant = QDateTime::fromSecsSinceEpoch(1700000000, QTimeZone::UTC);
    const QString result = TimestampConversion::toIso8601Local(utcInstant);

    QVERIFY(!result.isEmpty());

    const QDateTime parsedBack = TimestampConversion::fromIso8601(result);
    QVERIFY(parsedBack.isValid());
    QCOMPARE(parsedBack, utcInstant);
}

void TestTimestampConversion::test_toIso8601Local_invalid()
{
    const QString result = TimestampConversion::toIso8601Local(QDateTime());
    QVERIFY(result.isEmpty());
}

void TestTimestampConversion::test_roundTrip_allRepresentations()
{
    const QDateTime original =
        TimestampConversion::fromUnixTimestamp("1700000000", TimestampConversion::Unit::Seconds);
    QVERIFY(original.isValid());

    const QString iso = TimestampConversion::toIso8601(original);
    const QDateTime fromIso = TimestampConversion::fromIso8601(iso);
    QVERIFY(fromIso.isValid());
    QCOMPARE(fromIso, original);

    const QString seconds =
        TimestampConversion::toUnixTimestamp(fromIso, TimestampConversion::Unit::Seconds);
    QCOMPARE(seconds, QStringLiteral("1700000000"));
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestTimestampConversion)

#include "test_timestamp_conversion.moc"
