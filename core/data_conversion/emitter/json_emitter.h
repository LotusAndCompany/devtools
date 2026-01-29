#ifndef JSON_EMITTER_H
#define JSON_EMITTER_H

#include "basic_emitter.h"

#ifdef _TEST_JsonEmitter
namespace Test {
class TestJsonEmitter;
}
#endif

/// QVariantをJSONにする
class JsonEmitter : BasicEmitter
{
public:
    JsonEmitter() = default;

    /**
     * @brief データを文字列に変換する
     * @param data 入力データ
     * @param indentation 出力時のインデント(一部非対応あり)
     * @return 結果
     */
    EmitResult emitQString(const QVariant &data, DataConversion::Indentation indentation);

private:
#ifdef _TEST_JsonEmitter
    friend class Test::TestJsonEmitter;
#endif
};

#endif // JSON_EMITTER_H
