#include "tests/test_util.h"

#include <QtTest>

#define _TEST_JsonEmitter
#include "core/data_conversion/emitter/json_emitter.h"
#undef _TEST_JsonEmitter

#include <QObject>

namespace Test {
class TestJsonEmitter : public QObject
{
    Q_OBJECT

    RandomData rd;

private slots:
    void test_emitQString();
};

void TestJsonEmitter::test_emitQString()
{
    JsonEmitter emitter;
    JsonEmitter::EmitResult result;

    // 配列を変換できること
    QVariantList list = QVariantList{
        QVariant::fromValue(nullptr),
        true,
        false,
        rd.nextInt(),
        1.25,
        rd.nextQString(RandomData::numbers + RandomData::lowerAlphabets +
                       RandomData::upperAlphabets),
        QVariantList(),
        QVariantMap(),
    };
    result = emitter.emitQString(list, DataConversion::Indentation::MINIFIED);
    QCOMPARE_EQ(result.text, QString("[null,true,false,%1,1.25,\"%2\",[],{}]")
                                 .arg(list[3].toInt())
                                 .arg(list[5].toString()));

    // 連想配列配列を変換できること
    QVariantMap map = QVariantMap{
        {"n", QVariant::fromValue(nullptr)},
        {"t", true},
        {"f", false},
        {"i", rd.nextInt()},
        {"d", 2.25},
        {"s", rd.nextQString(RandomData::numbers + RandomData::lowerAlphabets +
                             RandomData::upperAlphabets)},
        {"a", QVariantList()},
        {"o", QVariantMap()},
    };
    // FIXME: 順番が変わってしまう
    result = emitter.emitQString(map, DataConversion::Indentation::MINIFIED);
    QCOMPARE_EQ(result.text,
                QString("{\"a\":[],\"d\":2.25,\"f\":false,\"i\":%1,\"n\":null,\"o\":{},\"s\":\"%"
                        "2\",\"t\":true}")
                    .arg(map["i"].toInt())
                    .arg(map["s"].toString()));

    list = {0, 1};
    map = {{"a", 0}, {"b", 1}};

    // 4スペースで出力されること
    result = emitter.emitQString(list, DataConversion::Indentation::SPACES_4);
    QCOMPARE_EQ(result.text, QString("[\n    0,\n    1\n]\n"));
    result = emitter.emitQString(map, DataConversion::Indentation::SPACES_4);
    QCOMPARE_EQ(result.text, QString("{\n    \"a\": 0,\n    \"b\": 1\n}\n"));

    // 2スペースで出力されること
    result = emitter.emitQString(list, DataConversion::Indentation::SPACES_2);
    QCOMPARE_EQ(result.text, QString("[\n  0,\n  1\n]\n"));
    result = emitter.emitQString(map, DataConversion::Indentation::SPACES_2);
    QCOMPARE_EQ(result.text, QString("{\n  \"a\": 0,\n  \"b\": 1\n}\n"));

    // タブで出力されること
    result = emitter.emitQString(list, DataConversion::Indentation::TABS);
    QCOMPARE_EQ(result.text, QString("[\n\t0,\n\t1\n]\n"));
    result = emitter.emitQString(map, DataConversion::Indentation::TABS);
    QCOMPARE_EQ(result.text, QString("{\n\t\"a\": 0,\n\t\"b\": 1\n}\n"));

    // インデント無しで出力されること
    result = emitter.emitQString(list, DataConversion::Indentation::MINIFIED);
    QCOMPARE_EQ(result.text, QString("[0,1]"));
    result = emitter.emitQString(map, DataConversion::Indentation::MINIFIED);
    QCOMPARE_EQ(result.text, QString("{\"a\":0,\"b\":1}"));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestJsonEmitter)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
// QTEST_GUILESS_MAIN(Test::TestJsonEmitter)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
// QTEST_MAIN(Test::TestJsonEmitter)

#include "test_json_emitter.moc"
