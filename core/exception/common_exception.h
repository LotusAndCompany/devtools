#ifndef COMMON_EXCEPTION_H
#define COMMON_EXCEPTION_H

#include <QException>
#include <QTextStream>

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
    inline virtual void raise() const override { throw *this; }

protected:
    // NOTE: protectedにしたい
    virtual QException *clone() const override { return new CommonException(*this); }
};

// FIXME: qDebug()やqWarning()に出力できない
//        Loggerを作り、そこに出力できるようにする
inline QTextStream &operator<<(QTextStream &stream, const CommonException &exception)
{
    return stream << exception.message;
}
// NOTE: QDataStreamには出力しなそうなので未実装

#endif // COMMON_EXCEPTION_H
