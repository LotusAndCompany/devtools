#ifndef TOML_PARSER_H
#define TOML_PARSER_H

#include "basic_parser.h"

namespace toml {
struct type_config;

template<typename TC>
class basic_value;
}; // namespace toml

namespace _TomlParserPrivate {
struct Util;
};

#ifdef _TEST_TomlParser
namespace Test {
class TestTomlParser;
}
#endif

/**
 * @brief TOMLを解析してQVarintMap/QVarintList/QVariantにする
 */
class TomlParser : public BasicParser
{
public:
    ParseResult tryParse(const QString &src) const override;

private:
    /// tomlのデータ型
    using toml_value_type = toml::basic_value<toml::type_config>;
    /**
     * @brief tomlの値をQVariantに変換する
     * @param value 入力値
     * @return 変換後の値
     */
    static ParseResult tomlValueToQvariant(const toml_value_type &value);
    /**
     * @brief tomlの連想配列をQVariantMapに変換する
     * @param value 入力値
     * @return 変換後の値
     */
    static ParseResult tomlTableToQvariantMap(const toml_value_type &value);
    /**
     * @brief tomlの配列をQVariantListに変換する
     * @param value 入力値
     * @return 変換後の値
     */
    static ParseResult tomlArrayToQvariantList(const toml_value_type &value);

    friend struct _TomlParserPrivate::Util;
#ifdef _TEST_TomlParser
    friend class Test::TestTomlParser;
#endif
};

#endif // TOML_PARSER_H
