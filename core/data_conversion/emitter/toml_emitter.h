#ifndef TOML_EMITTER_H
#define TOML_EMITTER_H

#include "basic_emitter.h"

class TomlEmitter : public BasicEmitter
{
public:
    TomlEmitter(){}

    // BasicEmitter interface
public:
    QString emitQString(const QVariant &data,
                        DataConversionInterface::Indentation style) const override;
};

#endif // TOML_EMITTER_H
