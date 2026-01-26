#ifndef UNDER_DEVELOPMENT_EXCEPTION_H
#define UNDER_DEVELOPMENT_EXCEPTION_H

#include "common_exception.h"

#ifdef _TEST_UnderDevelopmentException
namespace Test {
class TestUnderDevelopmentException;
}
#endif

/**
 * @brief 未実装の部分がある場合に発生させる例外
 * @details 開発中に仕方無く発生させるものであって、リリース時にはこの例外が発生しないようにする
 */
class UnderDevelopmentException : public CommonException
{
public:
    /**
     * @brief デフォルトコンストラクタ
     * @details "[UnderDevelopmentException] Under development..."というメッセージが設定される
     */
    UnderDevelopmentException()
        : CommonException("[UnderDevelopmentException] Under development...")
    {}
    /**
     * @brief コピーコンストラクタ
     * @param src コピー元インスタンス
     */
    UnderDevelopmentException(const UnderDevelopmentException &src) = default;
    /**
     * @brief 任意のメッセージを設定できるコンストラクタ
     * @param message メッセージ
     */
    explicit UnderDevelopmentException(const QString &message) : CommonException(message) {}

    void raise() const override { throw *this; }

protected:
    [[nodiscard]] QException *clone() const override
    {
        return new UnderDevelopmentException(*this);
    }

#ifdef _TEST_UnderDevelopmentException
    friend class Test::TestUnderDevelopmentException;
#endif
};

#endif // UNDER_DEVELOPMENT_EXCEPTION_H
