#ifndef JSON_EMITTER_H
#define JSON_EMITTER_H

#include "basic_emitter.h"

/// QVariantをJSONにする
class JsonEmitter : public BasicEmitter
{
public:
    JsonEmitter() {}

    // BaseEmitter interface
public:
    QString emitQString(
        const QVariant &data,
        DataConversion::Indentation style = DataConversion::Indentation::SPACES_4) const override;
};

#endif // JSON_EMITTER_H
