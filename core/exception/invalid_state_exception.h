#ifndef INVALID_STATE_EXCEPTION_H
#define INVALID_STATE_EXCEPTION_H

#include <QTextStream>
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
        QTextStream stream;
        stream << "[InvalidStateException] actual:" << actual << ", expected:" << expected;
        message = stream.readAll();
    }

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new InvalidStateException(*this); }
};

#endif // INVALID_STATE_EXCEPTION_H
