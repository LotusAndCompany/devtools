#ifndef INVALID_ARGUMENT_EXCEPTION_H
#define INVALID_ARGUMENT_EXCEPTION_H

#include <QTextStream>
#include "common_exception.h"

template<typename T>
class InvalidArgumentException : public CommonException
{
public:
    explicit InvalidArgumentException() = default;
    explicit InvalidArgumentException(const InvalidArgumentException &src) = default;
    explicit InvalidArgumentException(const QString &message)
        : CommonException(message)
    {}
    InvalidArgumentException(const T &given, const QString &reason)
    {
        QTextStream stream;
        stream << "[InvalidArgumentException] given:" << given << ", reason:" << reason;
        message = stream.readAll();
    }

    // NOTE: 明示的に実装しなくても良いはず
    //virtual void raise() const override { throw *this; }
    QException *clone() const override { return new InvalidArgumentException(*this); }
};

#endif // INVALID_ARGUMENT_EXCEPTION_H
