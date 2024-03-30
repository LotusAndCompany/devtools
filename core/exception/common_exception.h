#ifndef COMMON_EXCEPTION_H
#define COMMON_EXCEPTION_H

#include <QException>

class QTextStream;

// メッセージを設定できる例外
class CommonException : public QException
{
public:
    explicit CommonException() = default;
    explicit CommonException(const CommonException &src) = default;
    explicit CommonException(const QString &message);

    QString message;

    // NOTE: 明示的に実装しなくても良いはず
    //virtual void raise() const override { throw *this; }
    // NOTE: これも明示的に実装しなくても良いかも知れない
    QException *clone() const override;
};

QTextStream &operator<<(QTextStream &, const CommonException &exception);
// NOTE: QDataStreamには出力しなそうなので未実装

#endif // COMMON_EXCEPTION_H
