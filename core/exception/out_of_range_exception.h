#ifndef OUT_OF_RANGE_EXCEPTION_H
#define OUT_OF_RANGE_EXCEPTION_H

#include "common_exception.h"

#ifdef _TEST_OutOfRangeException
namespace Test {
class TestOutOfRangeException;
}
#endif

/**
 * @brief 範囲外アクセスが発生した時に発生させる例外
 * @details これを発生させれば良いという事ではなく、これが発生するようなコードは直すべき
 *
 * @tparam T インデックスの型 QString型に変換可能である必要がある
 */
template <typename T>
class OutOfRangeException : public CommonException
{
public:
    /**
     * @brief デフォルトコンストラクタ
     */
    OutOfRangeException() = default;
    /**
     * @brief コピーコンストラクタ
     * @param src コピー元インスタンス
     */
    OutOfRangeException(const OutOfRangeException &src) = default;
    OutOfRangeException(OutOfRangeException &&src) noexcept = default;
    OutOfRangeException &operator=(const OutOfRangeException &src) = default;
    OutOfRangeException &operator=(OutOfRangeException &&src) noexcept = default;
    ~OutOfRangeException() override = default;
    /**
     * @brief 任意のメッセージを設定できるコンストラクタ
     * @param message メッセージ
     */
    explicit OutOfRangeException(const QString &message) : CommonException(message) {}
    /**
     * @brief インデックスとその最大値からメッセージを生成するコンストラクタ
     * @param actual そのインデックス
     * @param max インデックスの最大値
     *
     * @details "[OutOfRangeException] actual: <actual>, max: <max>" の形式のメッセージが設定される
     */
    OutOfRangeException(const T &actual, const T &max)
    {
        message = QString("[OutOfRangeException] actual: %1, max: %2").arg(actual).arg(max);
    }
    /**
     * @brief インデックスとその最大値・最小値からメッセージを生成するコンストラクタ
     * @param actual そのインデックス
     * @param min インデックスの最小値
     * @param max インデックスの最大値
     *
     * @details "[OutOfRangeException] actual: <actual>, min: <min>, max: <max>"
     * の形式のメッセージが設定される
     */
    OutOfRangeException(const T &actual, const T &min, const T &max)
    {
        message = QString("[OutOfRangeException] actual: %1, min: %2, max: %3")
                      .arg(actual)
                      .arg(min)
                      .arg(max);
    }

    void raise() const override { throw *this; }

protected:
    [[nodiscard]] QException *clone() const override { return new OutOfRangeException(*this); }

#ifdef _TEST_OutOfRangeException
    friend class Test::TestOutOfRangeException;
#endif
};

#endif // OUT_OF_RANGE_EXCEPTION_H
