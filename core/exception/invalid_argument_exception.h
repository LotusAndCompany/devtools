#ifndef INVALID_ARGUMENT_EXCEPTION_H
#define INVALID_ARGUMENT_EXCEPTION_H

#include <QTextStream>
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
        QTextStream stream;
        stream << "[InvalidArgumentException] given:" << given << ", reason:" << reason;
        message = stream.readAll();
    }

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new InvalidArgumentException(*this); }
};

#endif // INVALID_ARGUMENT_EXCEPTION_H
