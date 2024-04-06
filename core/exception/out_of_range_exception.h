#ifndef OUT_OF_RANGE_EXCEPTION_H
#define OUT_OF_RANGE_EXCEPTION_H

#include "common_exception.h"

#ifdef _TEST_OutOfRangeException
namespace Test {
class TestOutOfRangeException;
}
#endif

template<typename T>
class OutOfRangeException : public CommonException
{
public:
    OutOfRangeException() = default;
    OutOfRangeException(const OutOfRangeException &src) = default;
    explicit OutOfRangeException(const QString &message)
        : CommonException(message)
    {}
    OutOfRangeException(const T &actual, const T &max)
    {
        message = QString("[OutOfRangeException] actual: %1, max: %2").arg(actual).arg(max);
    }
    OutOfRangeException(const T &actual, const T &min, const T &max)
    {
        message = QString("[OutOfRangeException] actual: %1, min: %2, max: %3")
                      .arg(actual)
                      .arg(min)
                      .arg(max);
    }

    virtual void raise() const override { throw *this; }

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new OutOfRangeException(*this); }

#ifdef _TEST_OutOfRangeException
    friend class Test::TestOutOfRangeException;
#endif
};

#endif // OUT_OF_RANGE_EXCEPTION_H
