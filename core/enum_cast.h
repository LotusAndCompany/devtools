#ifndef ENUM_CAST_H
#define ENUM_CAST_H

/**
 * @file
 * @brief 整数→enumのキャスト
 * @details 整数型を(比較的)安全に最小値=MIN, 最大値=MAXかつ値が連続であるenum型に変換するテンプレートを提供する@n
 *          MIN, MAXへの変換は許可しない@n
 *          要件を満たさないenum型に対しては特殊化が必要
 */

#include "exception/invalid_argument_exception.h"
#include <type_traits>

/**
 * @brief 静的な整数→enumのキャスト
 * @details 整数リテラルやコンパイル時定数をenumの値に変換する@n
 *          不正な値が与えられたらコンパイルエラーになる@n
 *          データ型はintのみ対応@n
 *          技術的にはtemplate metaprogrammingを利用している。
 * 
@verbatim
EnumType e = static_enum_cast<EnumType, 10>::result;
@endverbatim
 * 
 * @tparam enum_type 変換先のenum型
 * @tparam value 変換元の整数
 * 
 * @sa enum_cast
 */
template<typename enum_type, int value>
struct static_enum_cast
{
    /**
     * @brief 変換結果(コンパイル時定数)
     */
    static constexpr const enum_type result = static_cast<enum_type>(value);

private:
    // 型チェック
    static_assert(std::is_enum<enum_type>::value, "enum_type is not enum");

    /// 最小値(比較用)
    static constexpr const int MIN_VALUE = static_cast<int>(enum_type::MIN);
    /// 最大値(比較用)
    static constexpr const int MAX_VALUE = static_cast<int>(enum_type::MAX);

    static_assert(MIN_VALUE < value && value < MAX_VALUE, "invalid cast");
};

/**
 * @brief 静的とは限らない整数→enumのキャスト
 * @details 整数型をenumの値に変換する@n
 *          不正な値が与えられたら例外を投げる@n
 *          技術的にはgeneric programmingとキャスト演算子のオーバーロードを利用している。
 * 
@verbatim
EnumType e = enum_cast<EnumType>(10);
@endverbatim
 * 
 * @tparam enum_type 変換先のenum型
 * @tparam int_type 変換元の整数型(デフォルトはint、基本的に変えなくて良い)
 * 
 * @sa static_enum_cast
 */
template<typename enum_type, typename int_type = int>
struct enum_cast
{
    /**
     * @brief コンストラクタ
     * @param value 変換する整数値
     * @exception InvalidArgumentException &lt;int_type&gt; value <= enum_type::MIN または enum_type::MAX <= value の場合
     */
    explicit enum_cast(int_type value) noexcept(false)
    {
        if (MIN_VALUE < value && value < MAX_VALUE) {
            result = static_cast<enum_type>(value);
        } else {
            // NOTE: 念の為MINを設定する
            result = enum_type::MIN;

            // NOTE: valueがコンパイル時定数の場合にはコンパイルエラーになるのが望ましいが、恐らく無理
            throw InvalidArgumentException<int_type>(value,
                                                     QString("value must be in range (%1, %2)")
                                                         .arg(MIN_VALUE)
                                                         .arg(MAX_VALUE));
        }
    }
    enum_cast(const enum_cast &) = delete;

    /**
     * @brief 変換先のenum型への暗黙変換
     */
    operator enum_type() const { return result; }

private:
    // 型チェック
    static_assert(std::is_enum<enum_type>::value, "enum_type is not enum");
    static_assert(std::is_integral<int_type>::value, "int_type is not integer");

    /// 最小値(比較用)
    static constexpr const int_type MIN_VALUE = static_cast<int_type>(enum_type::MIN);
    /// 最大値(比較用)
    static constexpr const int_type MAX_VALUE = static_cast<int_type>(enum_type::MAX);

    // NOTE: コンストラクタで三項演算子を使えばconstにできるはず
    /// 結果
    enum_type result;
};

#endif // ENUM_CAST_H
