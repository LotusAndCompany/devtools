#ifndef COMMON_EXCEPTION_H
#define COMMON_EXCEPTION_H

#include <QDebug>
#include <QException>

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

    virtual void raise() const override { throw *this; }

protected:
    // NOTE: protectedにしたい
    inline virtual QException *clone() const override { return new CommonException(*this); }

#ifdef _TEST_CommonException
    friend class Test::TestCommonException;
#endif
};

// NOTE: QDebugに出力する演算子オーバーロード
// qDebug(), qInfo(), qWarning()等が参照を返さないのでQDebugを受け渡す
inline QDebug operator<<(QDebug debug, const CommonException &exception)
{
    return debug << exception.message;
}

#endif // COMMON_EXCEPTION_H
