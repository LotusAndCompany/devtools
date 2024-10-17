#include "basic_parser.h"

#include <core/exception/invalid_argument_exception.h>

void BasicParser::validatePointer(void *p)
{
    if (!p)
        throw InvalidArgumentException<nullptr_t>("void *p must not be null");
}
