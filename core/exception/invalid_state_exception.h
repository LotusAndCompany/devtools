#ifndef INVALID_STATE_EXCEPTION_H
#define INVALID_STATE_EXCEPTION_H

#include "common_exception.h"

#ifdef _TEST_InvalidStateException
namespace Test {
class TestInvalidStateException;
}
#endif

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
        message = QString("[InvalidStateException] actual: ") % actual % QString(", expected: ")
                  % expected;
    }

    virtual void raise() const override { throw *this; }

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new InvalidStateException(*this); }

#ifdef _TEST_InvalidStateException
    friend class Test::TestInvalidStateException;
#endif
};

#endif // INVALID_STATE_EXCEPTION_H
