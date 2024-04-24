#ifndef MOCK_HELPER_H
#define MOCK_HELPER_H

/// mockクラスの実装に使えるmock関数オブジェクトっぽいものを定義する

#include <functional>
#include <type_traits>

namespace Test {
/**
 * @brief 基底クラス
 */
template<typename result_type, typename... arg_types>
class _FunctionMockBase
{
    using function_type = std::function<result_type(arg_types...)>;

public:
    /**
     * @brief func を呼び出し _count を一つ増やす
     * @param args 引数
     * @return func が設定されていれば func の返り値、設定されていなければデフォルトの値。
     */
    result_type invoke(arg_types... args)
    {
        _count++;

        if (hasFunction()) {
            return func(args...);
        } else {
            return defaultBehaviorImpl(args...);
        }
    }

    /**
     * @brief 関数呼び出し演算子のオーバーロード。 invoke() を呼び出しているだけ。
     * @param args 引数
     * @return func が設定されていれば func の返り値、設定されていなければデフォルトの値。
     */
    inline result_type operator()(arg_types... args) { return invoke(args...); }

    /**
     * @brief func が設定されているかを返す
     * @return 設定されていれば`true`
     */
    inline bool hasFunction() const { return (bool) func; }
    /**
     * @brief この関数オブジェクトが呼び出されたかを返す
     * @return 呼び出されていれば`true`
     */
    inline bool isInvoked() const { return 0 < _count; }
    /**
     * @brief この関数オブジェクトが呼び出された回数を返す
     * @return 呼び出された回数
     */
    inline int count() const { return _count; }
    /**
     * @brief 呼び出された回数を0に戻す
     */
    inline void resetCount() { _count = 0; }

    /**
     * @brief func を削除する
     * @param keepCount `true`なら _count を初期化しない
     */
    void clearFunction(bool keepCount = false)
    {
        if (!keepCount)
            resetCount();
        func = function_type();
    }

    /**
     * @brief func を設定する
     * @param keepCount `true`なら _count を初期化しない
     */
    void setFunction(const function_type &f, bool keepCount = false)
    {
        if (!keepCount)
            resetCount();
        func = f;
    }

protected:
    /**
     * @brief デフォルトコンストラクタ
     */
    _FunctionMockBase() {}
    /**
     * @brief func を設定するコンストラクタ
     * @param f 設定する関数
     */
    _FunctionMockBase(const function_type &f)
        : func(f)
    {}

    /// invoke() で呼ばれる関数
    function_type func;

    /**
     * @brief func が設定されていない時に呼ばれる処理。デフォルトの挙動。
     * @param args 引数
     * @return デフォルトの返り値
     */
    virtual result_type defaultBehaviorImpl(arg_types... args) = 0;

private:
    /// この関数オブジェクトが呼び出された回数
    int _count = 0;
};

/**
 * @brief 多くの場合に使える_FunctionMockBaseの実装
 */
template<typename result_type, typename... arg_types>
class FunctionMock : public _FunctionMockBase<result_type, arg_types...>
{
public:
    using function_type = std::function<result_type(arg_types...)>;

    FunctionMock()
        : _FunctionMockBase<result_type, arg_types...>()
    {}
    FunctionMock(const function_type &f)
        : _FunctionMockBase<result_type, arg_types...>(f)
    {}

protected:
    result_type defaultBehaviorImpl(arg_types... args) override
    {
        if constexpr (std::is_void<result_type>::value) {
            // void
            return;
        } else if constexpr (std::is_pointer<result_type>::value) {
            // pointer
            return nullptr;
        } else if constexpr (std::is_fundamental<result_type>::value) {
            // primitive types such as int, float, char, bool, etc...
            return static_cast<result_type>(0);
        } else {
            // other types except reference. default constructor required.
            return result_type();
        }
    }
};

/**
 * @brief 返り値が参照型の時のFunctionMockの特殊化
 */
template<typename result_type, typename... arg_types>
class FunctionMock<result_type &, arg_types...>
    : public _FunctionMockBase<result_type &, arg_types...>
{
public:
    using function_type = std::function<result_type &(arg_types...)>;

    FunctionMock()
        : _FunctionMockBase<result_type &, arg_types...>()
    {}
    FunctionMock(const function_type &f)
        : _FunctionMockBase<result_type &, arg_types...>(f)
    {}

protected:
    result_type &defaultBehaviorImpl(arg_types... args) override { return defaultReturnValue; }

private:
    // default constructor required
    /// デフォルトの返り値
    result_type defaultReturnValue;
};
}

#endif  // MOCK_HELPER_H
