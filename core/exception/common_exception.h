#ifndef COMMON_EXCEPTION_H
#define COMMON_EXCEPTION_H

#include <QException>
#include <QTextStream>

#ifdef _TEST_CommonException
namespace Test {
class TestCommonException;
}
#endif

// メッセージを設定できる例外
class CommonException : public QException
{
public:
    CommonException() = default;
    CommonException(const CommonException &src) = default;
    explicit CommonException(const QString &message)
        : message(message)
    {}

    QString message;

    // NOTE: overrideする必要があるかは微妙だが、念の為
    virtual void raise() const override { throw *this; }

protected:
    // NOTE: protectedにしたい
    inline virtual QException *clone() const override { return new CommonException(*this); }

#ifdef _TEST_CommonException
    friend class Test::TestCommonException;
#endif
};

// FIXME: qDebug()やqWarning()に出力できない
//        Loggerを作り、そこに出力できるようにする
inline QTextStream &operator<<(QTextStream &stream, const CommonException &exception)
{
    return stream << exception.message;
}
// NOTE: QDataStreamには出力しなそうなので未実装

#endif // COMMON_EXCEPTION_H
