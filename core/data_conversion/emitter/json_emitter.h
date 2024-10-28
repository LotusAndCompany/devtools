#ifndef JSON_EMITTER_H
#define JSON_EMITTER_H

#include "base_emitter.h"

/// QVariantをJSONにする
class JsonEmitter : public BaseEmitter
{
public:
    JsonEmitter() {}

    // BaseEmitter interface
public:
    QString emitQString(const QVariant &data, DataConversion::Indentation style) const override;
};

#endif // JSON_EMITTER_H
