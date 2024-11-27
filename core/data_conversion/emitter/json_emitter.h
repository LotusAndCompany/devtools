#ifndef JSON_EMITTER_H
#define JSON_EMITTER_H

#include "basic_emitter.h"

/// QVariantをJSONにする
class JsonEmitter : public BasicEmitter
{
public:
    JsonEmitter() {}

    /**
     * @brief データを文字列に変換する
     * @param data 入力データ
     * @param indentation 出力時のインデント(一部非対応あり)
     * @return 結果
     */
    EmitResult emitQString(const QVariant &data, DataConversion::Indentation indentation);
};

#endif // JSON_EMITTER_H
