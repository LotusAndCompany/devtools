#ifndef YAML_PARSER_H
#define YAML_PARSER_H

#include "basic_parser.h"

namespace YAML {
class Node;
};

#ifdef _TEST_YamlParser
namespace Test {
class TestYamlParser;
}
#endif

/**
 * @brief YAMLを解析してQVarintMap/QVarintList/QVariantにする
 */
class YamlParser : public BasicParser
{
public:
    /// YAMLスタイルを格納するためのキー。extrasで使う
    static const QString EXTRAS_YAML_STYLE;

    ParseResult tryParse(const QString &src) const override;

private:
    /**
     * @brief `node` が有効かを判定する。有効の場合はresultには何もしない。
     * @param node 対象のノード
     * @param result 結果の格納先
     * @return 有効なら `true`
     */
    static bool validateYamlNode(const YAML::Node &node, ParseResult *result = nullptr);
    /**
     * @brief nodeを `QVariant` に変換する
     * @param node 変換前の値
     * @return 変換後の値
     */
    static ParseResult yamlNodeToQVariant(const YAML::Node &node);
    /**
     * @brief scalarを `QVariant` に変換する
     * @param node 変換前の値
     * @exception InvalidArgumentException<bool> nodeが不正な場合
     * @return 変換後の値 (dataは基本的に `QString` )
     */
    static ParseResult yamlScalarToQVariant(const YAML::Node &node) noexcept(false);
    /**
     * @brief mapを `QVariantMap` に変換する
     * @param node 変換前の値
     * @exception InvalidArgumentException<bool> nodeが不正な場合
     * @return 変換後の値
     */
    static ParseResult yamlMapToQVariantMap(const YAML::Node &node) noexcept(false);
    /**
     * @brief mapを `QVariantList` に変換する
     * @param node 変換前の値
     * @exception InvalidArgumentException<bool> nodeが不正な場合
     * @return 変換後の値
     */
    static ParseResult yamlSequenceToQVariantList(const YAML::Node &node) noexcept(false);

#ifdef _TEST_YamlParser
    friend class Test::TestYamlParser;
#endif
};

#endif // YAML_PARSER_H
