#ifndef INVALID_ARGUMENT_EXCEPTION_H
#define INVALID_ARGUMENT_EXCEPTION_H

#include "common_exception.h"

template<typename T>
class InvalidArgumentException : public CommonException
{
public:
    InvalidArgumentException() = default;
    InvalidArgumentException(const InvalidArgumentException &src) = default;
    explicit InvalidArgumentException(const QString &message)
        : CommonException(message)
    {}
    InvalidArgumentException(const T &given, const QString &reason)
    {
        message = QString("[InvalidArgumentException] given: %1, reason: %2").arg(given).arg(reason);
    }

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new InvalidArgumentException(*this); }
};

#endif // INVALID_ARGUMENT_EXCEPTION_H
