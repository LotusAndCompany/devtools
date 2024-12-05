#include <QtTest>
#include "tests/test_util.h"

#define _TEST_YamlEmitter
#include "core/data_conversion/emitter/yaml_emitter.h"
#undef _TEST_YamlEmitter

#include <QObject>

namespace Test {
class TestYamlEmitter : public QObject
{
    Q_OBJECT

    RandomData rd;

private slots:
    void test_emitQString();
};

void TestYamlEmitter::test_emitQString(){
    YamlEmitter emitter;
    YamlEmitter::EmitResult result;

    // 配列を変換できること
    QVariantList list = QVariantList{
        QVariant::fromValue(nullptr),
        true,
        false,
        rd.nextInt(),
        1.25,
        rd.nextQString(RandomData::numbers + RandomData::lowerAlphabets
                       + RandomData::upperAlphabets),
        QVariantList(),
        QVariantMap(),
    };
    result = emitter.emitQString(list,
                                 DataConversion::Format::YAML_BLOCK,
                                 DataConversion::Indentation::SPACES_2);
    QCOMPARE_EQ(result.text,
                QString("- \"\"\n- true\n- false\n- %1\n- 1.25\n- %2\n-\n  []\n- {}\n")
                    .arg(list[3].toInt())
                    .arg(list[5].toString()));
    // 連想配列配列を変換できること
    QVariantMap map = QVariantMap{
        {"n", QVariant::fromValue(nullptr)},
        {"t", true},
        {"f", false},
        {"i", rd.nextInt()},
        {"d", 2.25},
        {"s",
         rd.nextQString(RandomData::numbers + RandomData::lowerAlphabets
                        + RandomData::upperAlphabets)},
        {"a", QVariantList()},
        {"o", QVariantMap()},
    };
    // FIXME: 順番が変わってしまう
    result = emitter.emitQString(map,
                                 DataConversion::Format::YAML_BLOCK,
                                 DataConversion::Indentation::SPACES_2);
    QCOMPARE_EQ(result.text,
                QString("a:\n  []\nd: 2.25\nf: false\ni: %1\nn: \"\"\no:\n  {}\ns: %2\nt: true\n")
                    .arg(map["i"].toInt())
                    .arg(map["s"].toString()));

    list = {0, QVariantMap{{"a", 1}, {"b", 2}}};
    map = {{"a", 0}, {"b", QVariantMap{{"c", 1}, {"d", 2}}}};

    // 4スペースで出力されること
    result = emitter.emitQString(list,
                                 DataConversion::Format::YAML_BLOCK,
                                 DataConversion::Indentation::SPACES_4);
    QCOMPARE_EQ(result.text, QString("-   0\n-   a:  1\n    b:  2\n"));
    result = emitter.emitQString(map,
                                 DataConversion::Format::YAML_BLOCK,
                                 DataConversion::Indentation::SPACES_4);
    QCOMPARE_EQ(result.text, QString("a:  0\nb:\n    c:  1\n    d:  2\n"));

    // 2スペースで出力されること
    result = emitter.emitQString(list,
                                 DataConversion::Format::YAML_BLOCK,
                                 DataConversion::Indentation::SPACES_2);
    QCOMPARE_EQ(result.text, QString("- 0\n- a: 1\n  b: 2\n"));
    result = emitter.emitQString(map,
                                 DataConversion::Format::YAML_BLOCK,
                                 DataConversion::Indentation::SPACES_2);
    QCOMPARE_EQ(result.text, QString("a: 0\nb:\n  c: 1\n  d: 2\n"));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestYamlEmitter)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestYamlEmitter)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_MAIN(Test::TestYamlEmitter)

#include "test_yaml_emitter.moc"
