#ifndef INVALID_STATE_EXCEPTION_H
#define INVALID_STATE_EXCEPTION_H

#include "common_exception.h"

#ifdef _TEST_InvalidStateException
namespace Test {
class TestInvalidStateException;
}
#endif

/**
 * @brief 不正な状態になった時に発生させる例外
 * @details これを発生させれば良いという事ではなく、これが発生するようなコードは直すべき
 */
class InvalidStateException : public CommonException
{
public:
    /**
     * @brief デフォルトコンストラクタ
     */
    InvalidStateException() = default;
    /**
     * @brief コピーコンストラクタ
     * @param src コピー元インスタンス
     */
    InvalidStateException(const InvalidStateException &src) = default;
    /**
     * @brief 任意のメッセージを設定できるコンストラクタ
     * @param message メッセージ
     */
    explicit InvalidStateException(const QString &message)
        : CommonException(message)
    {}
    /**
     * @brief その状態と理由からメッセージを生成するコンストラクタ
     * @param actual その引数
     * @param expected 不正な理由、正しい状態など
     * 
     * @details "[InvalidStateException] actual: <actual>, expected: <expected>" の形式のメッセージが設定される
     */
    InvalidStateException(const QString &actual, const QString &expected)
    {
        message = QString("[InvalidStateException] actual: ") % actual % QString(", expected: ")
                  % expected;
    }

    /**
     * @brief このインスタンスを投げる
     * 
     * @sa [QException::raise()](https://doc.qt.io/qt-6/qexception.html#raise)
     */
    virtual void raise() const override { throw *this; }

protected:
    /**
     * @brief このインスタンスをコピーする
     * @return コピーされたインスタンス
     * @note この関数で生成されたインスタンスは`delete`等、何らかの方法で解放する必要がある
     * 
     * @sa [QException::clone()](https://doc.qt.io/qt-6/qexception.html#clone)
     */
    virtual QException *clone() const override { return new InvalidStateException(*this); }

#ifdef _TEST_InvalidStateException
    friend class Test::TestInvalidStateException;
#endif
};

#endif // INVALID_STATE_EXCEPTION_H
