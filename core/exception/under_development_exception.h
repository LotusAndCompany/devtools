#ifndef UNDER_DEVELOPMENT_EXCEPTION_H
#define UNDER_DEVELOPMENT_EXCEPTION_H

#include "common_exception.h"

// NOTE: 開発中以外は使わないように
class UnderDevelopmentException : public CommonException
{
public:
    UnderDevelopmentException()
        : CommonException("Under development...")
    {}
    UnderDevelopmentException(const UnderDevelopmentException &src) = default;
    explicit UnderDevelopmentException(const QString &message)
        : CommonException(message)
    {}

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new UnderDevelopmentException(*this); }
};

#endif // UNDER_DEVELOPMENT_EXCEPTION_H
