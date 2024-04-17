#ifndef OUT_OF_RANGE_EXCEPTION_H
#define OUT_OF_RANGE_EXCEPTION_H

#include <QTextStream>
#include "common_exception.h"

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
        QTextStream stream;
        stream << "[OutOfRangeException] actual:" << actual << ", max:" << max;
        message = stream.readAll();
    }
    OutOfRangeException(const T &actual, const T &min, const T &max)
    {
        QTextStream stream;
        stream << "[OutOfRangeException] actual:" << actual << "min:" << min << ", max:" << max;
        message = stream.readAll();
    }

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new OutOfRangeException(*this); }
};

#endif // OUT_OF_RANGE_EXCEPTION_H
