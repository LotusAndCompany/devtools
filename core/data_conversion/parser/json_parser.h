#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "basic_parser.h"

/**
 * @brief JSONを解析してQVarintMap/QVarintList/QVariantにする
 */
class JsonParser : public BasicParser
{
public:
    ParseResult tryParse(const QString &src) const override;
};

#endif // JSON_PARSER_H
