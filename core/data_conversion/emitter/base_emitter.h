#ifndef BASE_EMITTER_H
#define BASE_EMITTER_H

#include <QString>
#include "core/data_conversion/data_conversion.h"

/// データを文字列に変換するクラスの基底クラス
class BaseEmitter
{
public:
    /// デストラクタ
    virtual ~BaseEmitter() = default;

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

#endif // BASE_EMITTER_H
