#include <QtTest>
#include "core/exception/invalid_state_exception.h"
#include "tests/test_util.h"

#define _TEST_YamlParser
#include "core/data_conversion/parser/yaml_parser.h"
#undef _TEST_YamlParser

#include <QObject>
#include <yaml-cpp/yaml.h>

namespace Test {
class TestYamlParser : public QObject
{
    Q_OBJECT

    RandomData rd;

private slots:
    void test_tryParse();
    void test_validateYamlNode();
    void test_yamlNodeToQVariant();
    void test_yamlScalarToQVariant();
    void test_yamlMapToQVariantMap();
    void test_yamlSequenceToQVariantList();
};

void TestYamlParser::test_tryParse()
{
    // TODO: データ型の判別ができるようになったら直す

    YamlParser yp;

    // flow styleのyamlを読み込めること
    QFile file(TEST_SRC_DIR + "/core/data_conversion/test_flow.yaml");
    if (file.open(QIODevice::ReadOnly)) {
        const QVariantMap data = {
            {"null_key", QVariant::fromValue(nullptr)},
            {"integer_key", "42"},
            {"float_key", "2.7818281828"},
            {"string_key", "value"},
            {"true_key", "true"},
            {"false_key", "false"},
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
                    "-0.5",
                    "0",
                    "0.5",
                },
            },
        };

        QTextStream stream(&file);
        const QString yaml = stream.readAll();

        const auto result = yp.tryParse(yaml);
        // mapの解析が成功すること
        QVERIFY(result.success);
        QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::MAP);
        QVERIFY(result.errors.empty());
        // flow styleであること
        QVERIFY(result.extras.contains(YamlParser::EXTRAS_YAML_STYLE));
        QCOMPARE_EQ(result.extras[YamlParser::EXTRAS_YAML_STYLE],
                    static_cast<int>(YAML::EmitterStyle::Flow));

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
    file.close();

    // block styleのyamlを読み込めること
    file.setFileName(TEST_SRC_DIR + "/core/data_conversion/test_block.yaml");
    if (file.open(QIODevice::ReadOnly)) {
        const QVariantMap data = {
            {"null_key", QVariant::fromValue(nullptr)},
            {"integer_key", "42"},
            {"float_key", "2.7818281828"},
            {"string_key", "value"},
            {"true_key", "true"},
            {"false_key", "false"},
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
                    "-0.5",
                    "0",
                    "0.5",
                },
            },
            {
                "multiline_strings",
                QVariantList{
                    "aaa bbb ccc",
                    "aaa bbb ccc",
                    "aaa\nbbb\nccc",
                    "aaa\nbbb\nccc\n",
                },
            },
        };

        QTextStream stream(&file);
        const QString yaml = stream.readAll();

        const auto result = yp.tryParse(yaml);
        // mapの解析が成功すること
        QVERIFY(result.success);
        QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::MAP);
        QVERIFY(result.errors.empty());
        // block styleであること
        QVERIFY(result.extras.contains(YamlParser::EXTRAS_YAML_STYLE));
        QCOMPARE_EQ(result.extras[YamlParser::EXTRAS_YAML_STYLE],
                    static_cast<int>(YAML::EmitterStyle::Block));

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

void TestYamlParser::test_validateYamlNode()
{
    YamlParser::ParseResult result;

    // NOTE: どのような場合に無効なノードができるのか分からないのでtrueを返すパターンのみ
    const auto scalarNode = YAML::Node("scalar");
    QVERIFY(scalarNode.IsScalar());

    // Scalarを渡すとtrueを返すこと
    QVERIFY(YamlParser::validateYamlNode(scalarNode));
    QVERIFY(YamlParser::validateYamlNode(scalarNode, &result));

    const auto sequenceNode = YAML::Node(std::vector<std::string>{"a", "b"});
    QVERIFY(sequenceNode.IsSequence());
    // Sequenceを渡すとtrueを返すこと
    QVERIFY(YamlParser::validateYamlNode(sequenceNode));
    QVERIFY(YamlParser::validateYamlNode(sequenceNode, &result));

    const auto mapNode = YAML::Node(std::map<std::string, std::string>{{"a", "b"}});
    QVERIFY(mapNode.IsMap());
    // Mapを渡すとtrueを返すこと
    QVERIFY(YamlParser::validateYamlNode(mapNode));
    QVERIFY(YamlParser::validateYamlNode(mapNode, &result));
}

void TestYamlParser::test_yamlNodeToQVariant()
{
    // Scalarを変換できること
    const auto scalarNode = YAML::Node("scalar");
    QVERIFY(scalarNode.IsScalar());
    auto result = YamlParser::yamlNodeToQVariant(scalarNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::VARIANT);

    // Sequenceを変換できること
    const auto sequenceNode = YAML::Node(std::vector<std::string>{"a", "b"});
    QVERIFY(sequenceNode.IsSequence());
    result = YamlParser::yamlNodeToQVariant(sequenceNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::LIST);

    // Mapを変換できること
    const auto mapNode = YAML::Node(std::map<std::string, std::string>{{"a", "b"}});
    QVERIFY(mapNode.IsMap());
    result = YamlParser::yamlNodeToQVariant(mapNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::MAP);
}

void TestYamlParser::test_yamlScalarToQVariant()
{
    // TODO: データ型の判別ができるようになったら直す

    // 文字列を変換できること
    auto scalarNode = YAML::Node("scalar");
    QVERIFY(scalarNode.IsScalar());

    auto result = YamlParser::yamlScalarToQVariant(scalarNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::VARIANT);
    QCOMPARE_EQ(result.data, "scalar");
    QVERIFY(result.errors.empty());

    // 整数値を変換できること
    scalarNode = YAML::Node(255);
    QVERIFY(scalarNode.IsScalar());

    result = YamlParser::yamlScalarToQVariant(scalarNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::VARIANT);
    QCOMPARE_EQ(result.data, "255");
    QVERIFY(result.errors.empty());

    // 小数値を変換できること
    scalarNode = YAML::Node(2.71828);
    QVERIFY(scalarNode.IsScalar());

    result = YamlParser::yamlScalarToQVariant(scalarNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::VARIANT);
    QCOMPARE_EQ(result.data, "2.71828");
    QVERIFY(result.errors.empty());

    // ScalarでないNodeが渡されると例外が発生すること
    QVERIFY_THROWS_EXCEPTION(InvalidStateException, YamlParser::yamlScalarToQVariant(YAML::Node()));
}

void TestYamlParser::test_yamlMapToQVariantMap()
{
    // TODO: データ型の判別ができるようになったら直す

    // mapを変換できること
    auto mapNode = YAML::Node(std::map<std::string, std::string>{});
    mapNode["str"] = rd.nextQString().toStdString();
    mapNode["int"] = rd.nextInt();
    mapNode["float"] = rd.nextDouble();
    QVERIFY(mapNode.IsMap());

    const auto result = YamlParser::yamlMapToQVariantMap(mapNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::MAP);
    QVERIFY(result.errors.empty());
    const auto resultMap = result.data.toMap();
    QCOMPARE_EQ(resultMap.size(), 3);
    QVERIFY(resultMap.contains("str"));
    QCOMPARE_EQ(resultMap["str"].toString().toStdString(), mapNode["str"].Scalar());
    QVERIFY(resultMap.contains("int"));
    QCOMPARE_EQ(resultMap["int"].toString().toStdString(), mapNode["int"].Scalar());
    QVERIFY(resultMap.contains("float"));
    QCOMPARE_EQ(resultMap["float"].toString().toStdString(), mapNode["float"].Scalar());

    // MapでないNodeが渡されると例外が発生すること
    QVERIFY_THROWS_EXCEPTION(InvalidStateException, YamlParser::yamlMapToQVariantMap(YAML::Node()));
}

void TestYamlParser::test_yamlSequenceToQVariantList()
{
    // TODO: データ型の判別ができるようになったら直す

    // sequenceを変換できること(int)
    auto sequenceNode = YAML::Node(std::vector<int>{
        -10,
        0,
        10,
    });
    QVERIFY(sequenceNode.IsSequence());
    auto result = YamlParser::yamlSequenceToQVariantList(sequenceNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::LIST);
    QVERIFY(result.errors.empty());
    auto resultList = result.data.toList();
    QCOMPARE_EQ(resultList.size(), 3);
    QCOMPARE_EQ(resultList[0].toString(), "-10");
    QCOMPARE_EQ(resultList[1].toString(), "0");
    QCOMPARE_EQ(resultList[2].toString(), "10");

    // sequenceを変換できること(float)
    sequenceNode = YAML::Node(std::vector<float>{
        -1.5,
        0,
        2.5,
    });
    QVERIFY(sequenceNode.IsSequence());
    result = YamlParser::yamlSequenceToQVariantList(sequenceNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::LIST);
    QVERIFY(result.errors.empty());
    resultList = result.data.toList();
    QCOMPARE_EQ(resultList.size(), 3);
    QCOMPARE_EQ(resultList[0].toString(), "-1.5");
    QCOMPARE_EQ(resultList[1].toString(), "0");
    QCOMPARE_EQ(resultList[2].toString(), "2.5");

    // sequenceを変換できること(string)
    sequenceNode = YAML::Node(std::vector<std::string>{
        rd.nextQString().toStdString(),
        rd.nextQString().toStdString(),
        rd.nextQString().toStdString(),
    });
    QVERIFY(sequenceNode.IsSequence());
    result = YamlParser::yamlSequenceToQVariantList(sequenceNode);
    QVERIFY(result.success);
    QCOMPARE_EQ(result.type, YamlParser::ParseResult::DataType::LIST);
    QVERIFY(result.errors.empty());
    resultList = result.data.toList();
    QCOMPARE_EQ(resultList.size(), 3);
    QCOMPARE_EQ(resultList[0].toString().toStdString(), sequenceNode[0].Scalar());
    QCOMPARE_EQ(resultList[1].toString().toStdString(), sequenceNode[1].Scalar());
    QCOMPARE_EQ(resultList[2].toString().toStdString(), sequenceNode[2].Scalar());
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestYamlParser)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestYamlParser)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_MAIN(Test::TestYamlParser)

#include "test_yaml_parser.moc"
