#ifndef INVALID_STATE_EXCEPTION_H
#define INVALID_STATE_EXCEPTION_H

#include "common_exception.h"

class InvalidStateException : public CommonException
{
public:
    InvalidStateException() = default;
    InvalidStateException(const InvalidStateException &src) = default;
    explicit InvalidStateException(const QString &message)
        : CommonException(message)
    {}
    InvalidStateException(const QString &actual, const QString &expected)
    {
        message = QString("[InvalidStateException] actual: %1, expected: %2").arg(actual, expected);
    }

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new InvalidStateException(*this); }
};

#endif // INVALID_STATE_EXCEPTION_H
