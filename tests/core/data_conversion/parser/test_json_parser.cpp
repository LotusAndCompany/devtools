#include <QtTest>
#include "tests/test_util.h"

#define _TEST_JsonParser
#include "core/data_conversion/parser/json_parser.h"
#undef _TEST_JsonParser

#include <QObject>
#include <QVariantMap>
#include <core/exception/invalid_argument_exception.h>

namespace Test {
class TestJsonParser : public QObject
{
    Q_OBJECT

private slots:
    void test_tryParse();
};

void TestJsonParser::test_tryParse()
{
    JsonParser jp;
    JsonParser::ParseResult result;

    result = jp.tryParse("");
    // 空文字列の解析が成功すること
    QVERIFY(result.success);
    //QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QVariant);
    QCOMPARE_EQ(result.type, JsonParser::ParseResult::DataType::VARIANT);
    QCOMPARE_EQ(result.data, QVariant());
    QVERIFY(result.errors.empty());

    const QVariantMap data = {
        {"null_key", QVariant::fromValue(nullptr)},
        {"integer_key", 42},
        {"float_key", 2.7818281828},
        {"string_key", "value"},
        {"true_key", true},
        {"false_key", false},
        {
            "map_key",
            QVariantMap{
                {
                    "key",
                    "v",
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
    };

    QFile file(TEST_SRC_DIR + "/core/data_conversion/test.json");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        const QString json = stream.readAll();

        result = jp.tryParse(json);
        // mapの解析が成功すること
        QVERIFY(result.success);
        QCOMPARE_EQ(result.data.typeId(), QMetaType::Type::QVariantMap);
        QCOMPARE_EQ(result.type, JsonParser::ParseResult::DataType::MAP);
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
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestJsonParser)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestJsonParser)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_MAIN(Test::TestJsonParser)

#include "test_json_parser.moc"
