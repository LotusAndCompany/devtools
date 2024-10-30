#ifndef BASIC_EMITTER_H
#define BASIC_EMITTER_H

#include <QString>
#include "core/data_conversion/data_conversion.h"

#ifdef _TEST_BasicEmitter
namespace Test {
class TestBasicEmitter;
}
#endif

/// データを文字列に変換するクラスの基底クラス
class BasicEmitter
{
public:
    /// デストラクタ
    virtual ~BasicEmitter() = default;

    /**
     * @brief データを文字列に変換する
     * @param data 入力データ
     * @param style 出力スタイル(一部非対応あり)
     * @return 結果
     */
    virtual QString emitQString(const QVariant &data, DataConversion::Indentation style) const = 0;

protected:
    /**
     * @brief 4スペースのインデントを指定のインデントに置換する
     * @param src 入力
     * @param newIndent 置換先の文字列
     * @return 結果
     */
    virtual QString replace4spaceIndentation(const QString &src, const QString &newIndent) const;

#ifdef _TEST_BasicEmitter
    friend class Test::TestBasicEmitter;
#endif
};

#endif // BASIC_EMITTER_H
