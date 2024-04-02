#ifndef ENUM_CAST_H
#define ENUM_CAST_H

#include "exception/invalid_argument_exception.h"
#include <type_traits>

// NOTE: 整数型を(比較的)安全に最小値=MIN, 最大値=MAXかつ値が連続であるenum型に変換する
//       要件を満たさないenum型に対しては特殊化が必要

// 静的なキャスト
// 不正な値が与えられたらコンパイルエラーになる
template<typename enum_type, typename int_type = int>
struct static_enum_cast
{
    explicit static_enum_cast(int_type value)
        : result(static_cast<enum_type>(value))
    {
        static_assert(MIN_VALUE < value && value < MAX_VALUE, "invalid cast");
    }
    static_enum_cast(const static_enum_cast &) = delete;

    // NOTE: 暗黙変換を許可
    constexpr operator enum_type() const noexcept(true) { return result; }

private:
    // 型チェック
    static_assert(std::is_enum<enum_type>::value, "enum_type is not enum");
    static_assert(std::is_integral<int_type>::value, "int_type is not integer");

    static constexpr const int_type MIN_VALUE = static_cast<int_type>(enum_type::MIN);
    static constexpr const int_type MAX_VALUE = static_cast<int_type>(enum_type::MAX);

    const enum_type result;
};

// 静的とは限らないキャスト
// 不正な値が与えられたら例外を投げる(InvalidArgumentException<int_type>)
template<typename enum_type, typename int_type = int>
struct enum_cast
{
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

    // NOTE: 暗黙変換を許可
    operator enum_type() const { return result; }

private:
    // 型チェック
    static_assert(std::is_enum<enum_type>::value, "enum_type is not enum");
    static_assert(std::is_integral<int_type>::value, "int_type is not integer");

    static constexpr const int_type MIN_VALUE = static_cast<int_type>(enum_type::MIN);
    static constexpr const int_type MAX_VALUE = static_cast<int_type>(enum_type::MAX);

    enum_type result;
};

#endif // ENUM_CAST_H
