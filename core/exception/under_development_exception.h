#ifndef UNDER_DEVELOPMENT_EXCEPTION_H
#define UNDER_DEVELOPMENT_EXCEPTION_H

#include "common_exception.h"

#ifdef _TEST_UnderDevelopmentException
namespace Test {
class TestUnderDevelopmentException;
}
#endif

// NOTE: 開発中以外は使わないように
class UnderDevelopmentException : public CommonException
{
public:
    UnderDevelopmentException()
        : CommonException("[UnderDevelopmentException] Under development...")
    {}
    UnderDevelopmentException(const UnderDevelopmentException &src) = default;
    explicit UnderDevelopmentException(const QString &message)
        : CommonException(message)
    {}

    virtual void raise() const override { throw *this; }

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new UnderDevelopmentException(*this); }

#ifdef _TEST_UnderDevelopmentException
    friend class Test::TestUnderDevelopmentException;
#endif
};

#endif // UNDER_DEVELOPMENT_EXCEPTION_H
