#ifndef TOML_EMITTER_H
#define TOML_EMITTER_H

#include "basic_emitter.h"

namespace toml {
struct type_config;

template <typename TC>
class basic_value;
}; // namespace toml

#ifdef _TEST_TomlEmitter
namespace Test {
class TestTomlEmitter;
}
#endif

namespace TomlEmitterPrivate {
struct Util;
};

/// QVariantをTOMLにする
class TomlEmitter : BasicEmitter
{
public:
    TomlEmitter() = default;

    /**
     * @brief データを文字列に変換する
     * @param data 入力データ
     * @param indentation 出力時のインデント(一部非対応あり)
     * @return 結果
     */
    EmitResult emitQString(const QVariant &data, DataConversionInterface::Indentation indentation =
                                                     DataConversion::Indentation::SPACES_4);

private:
    /// tomlのデータ型
    using toml_value_type = toml::basic_value<toml::type_config>;

    /**
     * @brief QVaritant型をtoml::basic_value<toml::type_config>型に変換する
     * @param key このvalueのkey
     * @param value QVaritant型の値
     * @param style 出力時のインデント(一部非対応あり)
     * @return toml::basic_value<toml::type_config>型の値
     * @exception InvalidArgumentException<int> tomlに変換できない値の場合
     */
    toml_value_type variantToTomlValue(
        const QString &key, const QVariant &value,
        DataConversionInterface::Indentation indentation) noexcept(false);
    /**
     * @brief QVariantList型をtoml::basic_value<toml::type_config>型に変換する
     * @param key このvalueのkey
     * @param list QVariantList型の値
     * @param style 出力時のインデント(一部非対応あり)
     * @return toml::basic_value<toml::type_config>型の値
     * @exception InvalidArgumentException<int> tomlに変換できない値の場合
     */
    toml_value_type listToTomlArray(
        const QString &key, const QVariantList &list,
        DataConversionInterface::Indentation indentation) noexcept(false);
    /**
     * @brief QVariantMap型をtoml::basic_value<toml::type_config>型に変換する
     * @param key このvalueのkey
     * @param map QVariantMap型の値
     * @param style 出力時のインデント(一部非対応あり)
     * @return toml::basic_value<toml::type_config>型の値
     * @exception InvalidArgumentException<int> tomlに変換できない値の場合
     */
    toml_value_type mapToTomlTable(
        const QString &key, const QVariantMap &map,
        DataConversionInterface::Indentation indentation) noexcept(false);

    friend struct TomlEmitterPrivate::Util;
#ifdef _TEST_TomlEmitter
    friend class Test::TestTomlEmitter;
#endif
};

#endif // TOML_EMITTER_H
