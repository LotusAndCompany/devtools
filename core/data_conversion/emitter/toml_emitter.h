#ifndef TOML_EMITTER_H
#define TOML_EMITTER_H

#include "basic_emitter.h"

namespace toml {
struct type_config;

template<typename TC>
class basic_value;
}; // namespace toml

#ifdef _TEST_TomlEmitter
namespace Test {
class TestTomlEmitter;
}
#endif

namespace _TomlEmitterPrivate {
struct Util;
};

class TomlEmitter : public BasicEmitter
{
public:
    TomlEmitter(){}

    // BasicEmitter interface
public:
    EmitResult emitQString(
        const QVariant &data,
        DataConversionInterface::Indentation style = DataConversion::Indentation::SPACES_4) override;

private:
    /// tomlのデータ型
    using toml_value_type = toml::basic_value<toml::type_config>;

    /**
     * @brief QVaritant型をtoml::basic_value<toml::type_config>型に変換する
     * @param value QVaritant型の値
     * @@param style 出力スタイル(一部非対応あり)
     * @return toml::basic_value<toml::type_config>型の値
     * @exception InvalidArgumentException<int> tomlに変換できない値の場合
     */
    toml_value_type variantToTomlValue(const QString &key,
                                       const QVariant &value,
                                       DataConversionInterface::Indentation style) noexcept(false);
    /**
     * @brief QVariantList型をtoml::basic_value<toml::type_config>型に変換する
     * @param list QVariantList型の値
     * @param style 出力スタイル(一部非対応あり)
     * @return toml::basic_value<toml::type_config>型の値
     * @exception InvalidArgumentException<int> tomlに変換できない値の場合
     */
    toml_value_type listToTomlArray(const QString &key,
                                    const QVariantList &list,
                                    DataConversionInterface::Indentation style) noexcept(false);
    /**
     * @brief QVariantMap型をtoml::basic_value<toml::type_config>型に変換する
     * @param map QVariantMap型の値
     * @param style 出力スタイル(一部非対応あり)
     * @return toml::basic_value<toml::type_config>型の値
     * @exception InvalidArgumentException<int> tomlに変換できない値の場合
     */
    toml_value_type mapToTomlTable(const QString &key,
                                   const QVariantMap &map,
                                   DataConversionInterface::Indentation style) noexcept(false);

    friend struct _TomlEmitterPrivate::Util;
#ifdef _TEST_TomlEmitter
    friend class Test::TestTomlEmitter;
#endif
};

#endif // TOML_EMITTER_H
