#ifndef YAML_EMITTER_H
#define YAML_EMITTER_H

#include "basic_emitter.h"

namespace YAML {
class Node;
};

#ifdef _TEST_YamlEmitter
namespace Test {
class TestYamlEmitter;
}
#endif

/// QVariantをYAMLにする
class YamlEmitter : public BasicEmitter
{
public:
    YamlEmitter() {}

    /**
     * @brief データを文字列に変換する
     * @param data 入力データ
     * @param style 出力スタイル
     * @param indentation 出力時のインデント(一部非対応あり)
     * @return 結果
     */
    EmitResult emitQString(const QVariant &data,
                           DataConversionInterface::Format style,
                           DataConversionInterface::Indentation indentation);

private:
    /**
     * @brief QVariantList型をYAML::Node型に変換する
     * @param value QVariant型の値
     * @param style 出力スタイル
     * @param indentation 出力時のインデント(一部非対応あり)
     * @return YAML::Node型の値
     * @exception InvalidArgumentException<int> yamlに変換できない値の場合
     */
    YAML::Node variantToYamlNode(const QVariant &value,
                                 DataConversionInterface::Format style,
                                 DataConversionInterface::Indentation indentation) noexcept(false);

    /**
     * @brief QVariantList型をYAML::Node型に変換する
     * @param variantList QVariantList型の値
     * @param style 出力スタイル
     * @param indentation 出力時のインデント(一部非対応あり)
     * @return YAML::Node型の値
     * @exception InvalidArgumentException<int> styleがYAML以外の場合
     */
    YAML::Node variantListToYamlSequence(const QVariantList &variantList,
                                         DataConversionInterface::Format style,
                                         DataConversionInterface::Indentation indentation);

    /**
     * @brief QVariantMap型をYAML::Node型に変換する
     * @param variantMap QVariantMap型の値
     * @param style 出力スタイル
     * @param indentation 出力時のインデント(一部非対応あり)
     * @return YAML::Node型の値
     * @exception InvalidArgumentException<int> styleがYAML以外の場合
     */
    YAML::Node variantMapToYamlMap(const QVariantMap &variantMap,
                                   DataConversionInterface::Format style,
                                   DataConversionInterface::Indentation indentation);

#ifdef _TEST_YamlEmitter
    friend class Test::TestYamlEmitter;
#endif
};

#endif // YAML_EMITTER_H
