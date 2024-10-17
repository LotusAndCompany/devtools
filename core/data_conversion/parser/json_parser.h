#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "basic_parser.h"

/**
 * @brief JSONを解析してQVarintMap/QVarintList/QVariantにする
 */
class JsonParser : public BasicParser
{
public:
    void tryParse(const QString &src, ParseResult *result) const override;
};

#endif // JSON_PARSER_H
