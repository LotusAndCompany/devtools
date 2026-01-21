#ifndef BASIC_EMITTER_H
#define BASIC_EMITTER_H

#include "core/data_conversion/data_conversion.h"

#include <QString>

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

    /// 出力結果を格納する構造体
    struct EmitResult
    {
        QString text;         ///< 結果
        QStringList warnings; ///< 警告
        QString error;        ///< エラー
    };

protected:
    /**
     * @brief 4スペースのインデントを指定のインデントに置換する
     * @param src 入力
     * @param newIndent 置換先の文字列
     * @return 結果
     */
    virtual QString replace4spaceIndentation(const QString &src, const QString &newIndent) const;

    /// 警告
    QStringList warnings;

#ifdef _TEST_BasicEmitter
    friend class Test::TestBasicEmitter;
#endif
};

#endif // BASIC_EMITTER_H
