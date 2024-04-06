#ifndef INVALID_ARGUMENT_EXCEPTION_H
#define INVALID_ARGUMENT_EXCEPTION_H

#include "common_exception.h"

#ifdef _TEST_InvalidArgumentException
namespace Test {
class TestInvalidArgumentException;
}
#endif

/**
 * @brief 引数が不正な時に発生させる例外
 * @details これを発生させれば良いという事ではなく、これが発生するようなコードは直すべき
 * 
 * @tparam T 引数の型 QString型に変換可能である必要がある
 */
template<typename T>
class InvalidArgumentException : public CommonException
{
public:
    /**
     * @brief デフォルトコンストラクタ
     */
    InvalidArgumentException() = default;
    /**
     * @brief コピーコンストラクタ
     * @param コピー元インスタンス
     */
    InvalidArgumentException(const InvalidArgumentException &src) = default;
    /**
     * @brief 任意のメッセージを設定できるコンストラクタ
     * @param message メッセージ
     */
    explicit InvalidArgumentException(const QString &message)
        : CommonException(message)
    {}
    /**
     * @brief その引数と理由からメッセージを生成するコンストラクタ
     * @param given その引数
     * @param reason 不正な理由など
     * 
     * @details "[InvalidArgumentException] given: <given>, reason: <reason>" の形式のメッセージが設定される
     */
    InvalidArgumentException(const T &given, const QString &reason)
    {
        message = QString("[InvalidArgumentException] given: %1, reason: %2").arg(given).arg(reason);
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
    virtual QException *clone() const override { return new InvalidArgumentException(*this); }

#ifdef _TEST_InvalidArgumentException
    friend class Test::TestInvalidArgumentException;
#endif
};

#endif // INVALID_ARGUMENT_EXCEPTION_H
