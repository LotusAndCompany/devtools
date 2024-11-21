#include <QtTest>
#include "tests/test_util.h"

#define _TEST_TomlParser
#include "core/data_conversion/parser/toml_parser.h"
#include "toml.hpp"
#undef _TEST_TomlParser

#include <QObject>

namespace Test {
class TestTomlParser : public QObject
{
    Q_OBJECT

    RandomData rd;
    QTime getRandomQTime();
    QDate getRandomQDate();
    QTimeZone getRandomQTimeZone();

private slots:
    void test_tryParse();
    void test_tomlValueToQVariant();
    void test_tomlTableToQVariantMap();
    void test_tomlArrayToQVariantList();
};

void TestTomlParser::test_tryParse()
{
    TomlParser tp;
    TomlParser::ParseResult result;

    const QVariantMap data = {
        {"integer_key", 42},
        {"float_key", 2.7818281828},
        {"string_key", "value"},
        {"true_key", true},
        {"false_key", false},
        {
            "table_key",
            QVariantMap{
                {"key", "v"},
                {
                    "inner",
                    QVariantMap{{"key2", "v2"}},
                },
            },
        },
        {
            "array_key",
            QVariantList{
                -0.5,
                0,
                0.5,
            },
        },
        {
            "inline_table_key",
            QVariantMap{
                {"a", 0},
                {"b", 1},
            },
        },
    };

    QFile file(TEST_SRC_DIR + "/core/data_conversion/test.toml");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        const QString toml = stream.readAll();

        result = tp.tryParse(toml);
        // tableの解析が成功すること
        QVERIFY(result);
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QVariantMap);
        QVERIFY(result.errors.empty());
        // 読み込んだ内容がdataと一致していること
        const auto map = result.data.toMap();
        QCOMPARE_EQ(map.size(), data.size());
        for (const auto &entry : map.asKeyValueRange()) {
            const QString &key = entry.first;
            const QVariant &value = entry.second;

            QVERIFY(map.contains(key) && data.contains(key));
            QCOMPARE_EQ(data[key], value);
        }
    }
}

QTime TestTomlParser::getRandomQTime()
{
    const uint8_t randomHour = rd.nextInt(0, 24), randomMin = rd.nextInt(0, 60),
                  randomSec = rd.nextInt(0, 60);
    const uint16_t randomMsec = rd.nextInt(0, 1000);

    return QTime(randomHour, randomMin, randomSec, randomMsec);
}

QDate TestTomlParser::getRandomQDate()
{
    const int16_t randomAD = rd.nextInt(1970, 3000);
    const uint8_t randomMon = rd.nextInt(1, 13), randomDay = rd.nextInt(1, 29);

    return QDate(randomAD, randomMon, randomDay);
}

QTimeZone TestTomlParser::getRandomQTimeZone()
{
    const int8_t randomHour = rd.nextInt(-12, 13), randomMin = rd.nextInt(-59, 60);

    return QTimeZone(60 * (60 * randomHour + randomMin));
}

void TestTomlParser::test_tomlValueToQVariant()
{
    TomlParser::ParseResult result;

    {
        const bool randomBool = rd.nextInt(2); // 0,1

        // bool型の変換が成功すること
        result = TomlParser::tomlValueToQvariant(TomlParser::toml_value_type(randomBool));
        QVERIFY(result);
        // NOTE: LongLongになるらしい
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::Bool);
        QCOMPARE_EQ(result.data.toBool(), randomBool);
    }

    {
        const int64_t randomInt = rd.nextInt();

        // int型の変換が成功すること
        result = TomlParser::tomlValueToQvariant(TomlParser::toml_value_type(randomInt));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::LongLong);
        QCOMPARE_EQ(result.data.toLongLong(), randomInt);
    }

    {
        const auto randomDouble = rd.nextDouble();

        // double型の変換が成功すること
        result = TomlParser::tomlValueToQvariant(TomlParser::toml_value_type(randomDouble));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::Double);
        QCOMPARE_EQ(result.data.toDouble(), randomDouble);
    }

    {
        const auto randomQString = rd.nextQString();

        // string型の変換が成功すること
        result = TomlParser::tomlValueToQvariant(
            TomlParser::toml_value_type(randomQString.toStdString()));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QString);
        QCOMPARE_EQ(result.data.toString(), randomQString);
    }

    {
        const auto randomQTime = getRandomQTime();
        QVERIFY(randomQTime.isValid());

        // local_time型の変換が成功すること
        result = TomlParser::tomlValueToQvariant(
            TomlParser::toml_value_type::local_time_type(randomQTime.hour(),
                                                         randomQTime.minute(),
                                                         randomQTime.second(),
                                                         randomQTime.msec()));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QTime);
        QCOMPARE_EQ(result.data.toTime(), randomQTime);

        // 境界値が正しいこと
        result = TomlParser::tomlValueToQvariant(
            TomlParser::toml_value_type::local_time_type(0, 0, 0, 0));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.toTime(), QTime(0, 0, 0, 0));

        result = TomlParser::tomlValueToQvariant(
            TomlParser::toml_value_type::local_time_type(23, 59, 59, 999));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.toTime(), QTime(23, 59, 59, 999));
    }

    {
        const auto randomQDate = getRandomQDate();
        QVERIFY(randomQDate.isValid());

        // local_date型の変換が成功すること
        result = TomlParser::tomlValueToQvariant(
            TomlParser::toml_value_type::local_date_type(randomQDate.year(),
                                                         static_cast<toml::month_t>(
                                                             randomQDate.month() - 1),
                                                         randomQDate.day()));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QDate);
        QCOMPARE_EQ(result.data.toDate(), randomQDate);

        // 境界値
        result = TomlParser::tomlValueToQvariant(
            TomlParser::toml_value_type::local_date_type(1970, static_cast<toml::month_t>(0), 1));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.toDate(), QDate(1970, 1, 1));

        result = TomlParser::tomlValueToQvariant(
            TomlParser::toml_value_type::local_date_type(2999, static_cast<toml::month_t>(11), 31));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.toDate(), QDate(2999, 12, 31));
    }

    {
        const auto randomQTime = getRandomQTime();
        const auto randomQDate = getRandomQDate();

        const auto randomQDateTime = QDateTime(randomQDate, randomQTime);
        QVERIFY(randomQDateTime.isValid());

        result = TomlParser::tomlValueToQvariant(TomlParser::toml_value_type::local_datetime_type(
            TomlParser::toml_value_type::local_date_type(randomQDate.year(),
                                                         static_cast<toml::month_t>(
                                                             randomQDate.month() - 1),
                                                         randomQDate.day()),
            TomlParser::toml_value_type::local_time_type(randomQTime.hour(),
                                                         randomQTime.minute(),
                                                         randomQTime.second(),
                                                         randomQTime.msec())));
        // local_datetime型の変換が成功すること
        QVERIFY(result);
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QDateTime);
        QCOMPARE_EQ(result.data.toDateTime(), randomQDateTime);

        // 境界値
        result = TomlParser::tomlValueToQvariant(TomlParser::toml_value_type::local_datetime_type(
            TomlParser::toml_value_type::local_date_type(1970, static_cast<toml::month_t>(0), 1),
            TomlParser::toml_value_type::local_time_type(0, 0, 0, 0)));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.toDateTime(), QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0, 0)));

        result = TomlParser::tomlValueToQvariant(TomlParser::toml_value_type::local_datetime_type(
            TomlParser::toml_value_type::local_date_type(2999, static_cast<toml::month_t>(11), 31),
            TomlParser::toml_value_type::local_time_type(23, 59, 59, 999)));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.toDateTime(),
                    QDateTime(QDate(2999, 12, 31), QTime(23, 59, 59, 999)));
    }

    {
        const auto randomQTime = getRandomQTime();
        const auto randomQDate = getRandomQDate();
        const int8_t randomHourOffset = rd.nextInt(-12, 13), randomMinOffset = rd.nextInt(-59, 60);
        const auto randomQTimeZone = QTimeZone(60 * (60 * randomHourOffset + randomMinOffset));

        const auto randomQDateTime = QDateTime(randomQDate, randomQTime, randomQTimeZone);
        QVERIFY(randomQDateTime.isValid());

        result = TomlParser::tomlValueToQvariant(TomlParser::toml_value_type::offset_datetime_type(
            TomlParser::toml_value_type::local_date_type(randomQDate.year(),
                                                         static_cast<toml::month_t>(
                                                             randomQDate.month() - 1),
                                                         randomQDate.day()),
            TomlParser::toml_value_type::local_time_type(randomQTime.hour(),
                                                         randomQTime.minute(),
                                                         randomQTime.second(),
                                                         randomQTime.msec()),
            toml::time_offset(randomHourOffset, randomMinOffset)));
        // offset_datetime型の変換が成功すること
        QVERIFY(result);
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QDateTime);
        QCOMPARE_EQ(result.data.toDateTime(), randomQDateTime);

        // 境界値 ※ 60*(60*12+59)=46740
        result = TomlParser::tomlValueToQvariant(TomlParser::toml_value_type::offset_datetime_type(
            TomlParser::toml_value_type::local_date_type(1970, static_cast<toml::month_t>(0), 1),
            TomlParser::toml_value_type::local_time_type(0, 0, 0, 0),
            toml::time_offset(12, 59)));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.toDateTime(),
                    QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0, 0), QTimeZone(46740)));

        result = TomlParser::tomlValueToQvariant(TomlParser::toml_value_type::offset_datetime_type(
            TomlParser::toml_value_type::local_date_type(2999, static_cast<toml::month_t>(11), 31),
            TomlParser::toml_value_type::local_time_type(23, 59, 59, 999),
            toml::time_offset(-12, -59)));
        QVERIFY(result);
        QCOMPARE_EQ(result.data.toDateTime(),
                    QDateTime(QDate(2999, 12, 31), QTime(23, 59, 59, 999), QTimeZone(-46740)));
    }
}

void TestTomlParser::test_tomlTableToQVariantMap()
{
    auto table = std::unordered_map<std::string, TomlParser::toml_value_type>{
        {"str", TomlParser::toml_value_type(rd.nextQString().toStdString())},
        {"int", TomlParser::toml_value_type(rd.nextInt())},
        {"float", TomlParser::toml_value_type(rd.nextDouble())},
    };
    const auto tomlTable = TomlParser::toml_value_type(table);

    const auto result = TomlParser::tomlTableToQvariantMap(tomlTable);

    // 変換が成功すること
    QVERIFY(result);
    QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QVariantMap);
    const auto map = result.data.toMap();

    // 内容が一致していること
    QCOMPARE_EQ(map.size(), 3);
    QVERIFY(map.contains("str"));
    QCOMPARE_EQ(map["str"].toString().toStdString(), table["str"].as_string());
    QVERIFY(map.contains("int"));
    QCOMPARE_EQ(map["int"].toInt(), table["int"].as_integer());
    QVERIFY(map.contains("float"));
    QVERIFY(isEqaulApprox(map["float"].toFloat(), table["float"].as_floating()));
}

void TestTomlParser::test_tomlArrayToQVariantList()
{
    const auto array = std::vector<TomlParser::toml_value_type>{
        TomlParser::toml_value_type(rd.nextInt()),
        TomlParser::toml_value_type(rd.nextInt()),
        TomlParser::toml_value_type(rd.nextInt()),
    };
    const auto tomlArray = TomlParser::toml_value_type(array);

    const auto result = TomlParser::tomlArrayToQvariantList(tomlArray);

    // 変換が成功すること
    QVERIFY(result);
    QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QVariantList);
    const auto &list = result.data.toList();

    // 内容が一致していること
    QCOMPARE_EQ(result.data.toList().size(), 3);
    for (int i = 0; i < 3; i++) {
        QCOMPARE_EQ(list[i].toInt(), array[i].as_integer());
    }
}

} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestTomlParser)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestTomlParser)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_MAIN(Test::TestTomlParser)

#include "test_toml_parser.moc"
