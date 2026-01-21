#ifndef MOCK_HELPER_H
#define MOCK_HELPER_H

/// mockクラスの実装に使える、内容を差し替えられる関数オブジェクトっぽいものを定義する

#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Test {

template <typename>
class FunctionMock;

template <typename result_type>
struct _DefaultValue;

template <typename... arg_types>
struct _ArgumentsHistory;

/**
 * @brief mockの実装に使える、中身を入れ替えられる関数オブジェクト
 * @tparam result_type 関数の返り値の型
 * @tparam arg_types 関数の引数の型
 * @details 呼び出された回数と引数リストの履歴を記録できる。@n
 *          テンプレートパラメータは std::function<> と同じように result_type(arg_types...)
 * という形で渡す。
 *
 * @code
 * // declarations
 * FunctionMock<bool(int)> mock_isGraterThan0;  // always returns false
 * FunctionMock<int(int, int)> mock_add([](int a, int b){ return 100; }); // always returns 100
 * FunctionMock<void()> mock_process(printHello); // function pointer is also available. you should
 * define `void printHello()` somewhere.
 *
 * // mock implementation
 * bool isGraterThan0(int value) const override { return mock_isGraterThan0(value); }   // or
 * mock_isGraterThan0.invoke(value); int add(int a, int b) const override { return mock_add(a, b); }
 * // or mock_add.invoke(a, b); void process(void) override { mock_process(); }  // or
 * mock_process.invoke();
 *
 * // mock function returns FunctionMock's return value
 * isGraterThan0(10) == false;  // always returns 0, false, nullpter etc... by default
 * isGraterThan0(-10) == false;
 *
 * // change mock behavior. count will be reset by default.
 * mock_isGraterThan0.setFunction([](int value){ return true; });   // always returns true
 * isGraterThan0(10) == true;
 * isGraterThan0(-10) == true;
 * @endcode
 */
template <typename result_type, typename... arg_types>
class FunctionMock<result_type(arg_types...)>
{
    using function_type = std::function<result_type(arg_types...)>;
    using history_type = typename _ArgumentsHistory<arg_types...>::element_type;

public:
    /**
     * @brief デフォルトコンストラクタ
     */
    FunctionMock() {}
    /**
     * @brief func を設定するコンストラクタ
     * @param f 設定する関数。 std::function<return_type(arg_types...)> 型の値を受け取れる。
     */
    FunctionMock(const function_type &f) : func(f) {}
    /**
     * @brief func を呼び出し _count を一つ増やす。引数があれば argsHistory に追加する。
     * @param args 引数リスト
     * @return func が設定されていれば func の返り値、設定されていなければデフォルトの値。
     */
    result_type invoke(arg_types... args)
    {
        _count++;

        if constexpr (!std::is_void<history_type>::value) {
            argsHistory.push_back({args...});
        }

        if (hasFunction()) {
            return func(args...);
        } else {
            return _DefaultValue<result_type>::get();
        }
    }

    /**
     * @brief 関数呼び出し演算子のオーバーロード。 invoke() を呼び出しているだけ。
     * @param args 引数リスト
     * @return func が設定されていれば func の返り値、設定されていなければデフォルトの値。
     */
    inline result_type operator()(arg_types... args) { return invoke(args...); }

    /**
     * @brief func が設定されているかを返す
     * @return 設定されていれば`true`
     */
    inline bool hasFunction() const { return (bool)func; }
    /**
     * @brief この関数オブジェクトが呼び出されたかを返す
     * @return 呼び出されていれば`true`
     */
    inline bool isInvoked() const { return 0 < _count; }
    /**
     * @brief この関数オブジェクトが呼び出された回数を返す
     * @return 呼び出された回数
     */
    inline size_t count() const { return _count; }
    /**
     * @brief 呼び出された回数と引数リストの履歴を初期化する
     */
    void resetCount()
    {
        _count = 0;
        if constexpr (!std::is_void<history_type>::value) {
            argsHistory.clear();
        }
    }

    /**
     * @brief func を設定する
     * @param f 設定する関数。 std::function<return_type(arg_types...)> 型の値を受け取れる。
     * @param keepCount `true`なら _count と argsHistory とを初期化しない
     */
    void setFunction(const function_type &f, bool keepCount = false)
    {
        if (!keepCount)
            resetCount();

        func = f;
    }

    /**
     * @brief func を削除する
     * @param keepCount `true`なら _count と argsHistory とを初期化しない
     */
    inline void clearFunction(bool keepCount = false) { setFunction(function_type(), keepCount); }

    /**
     * @brief この関数オブジェクトに渡された引数リストの履歴を返す
     * @param index インデックス。0=最古、size()-1または-1=最新。
     * @return 渡された引数リストの履歴
     */
    inline const history_type &argumentsHistory(int index = -1) const
    {
        if constexpr (std::is_void<history_type>::value) {
            // void
            return;
        } else {
            return argsHistory[index];
        }
    }

private:
    /// invoke() で呼ばれる関数
    function_type func;

    /// この関数オブジェクトが呼び出された回数
    size_t _count = 0;

    /// この関数オブジェクトに渡された引数の履歴
    _ArgumentsHistory<arg_types...> argsHistory;
};

/**
 * @brief 対象の型のデフォルト値を取得する
 * @tparam result_type 関数の返り値の型
 */
template <typename result_type>
struct _DefaultValue
{
    /**
     * @brief デフォルト値を返す
     * @return デフォルト値
     */
    inline static result_type get()
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
 * @brief 参照型の場合の特殊化
 * @warning 返り値を共有しているので値を編集すると他の関数の返り値にも影響する
 */
template <typename result_type>
struct _DefaultValue<result_type &>
{
    /**
     * @brief デフォルト値の参照を返す
     * @return デフォルト値の参照
     */
    inline static result_type &get() { return value; }

private:
    /// デフォルト値(デフォルトコンストラクタが必要)
    static result_type value;
};

template <typename result_type>
result_type _DefaultValue<result_type &>::value;

/**
 * @brief 引数リストの履歴
 * @tparam arg_types 引数の型
 */
template <typename... arg_types>
struct _ArgumentsHistory
{
    // NOTE: 工夫すれば引数が一つの時はtupleにしないようにもできそう
    using element_type = std::tuple<arg_types...>;

    /**
     * @brief 初期化する
     */
    inline void clear() { history.clear(); }
    /**
     * @brief 指定のインデックスの履歴を返す
     * @param index インデックス。0=最古、size()-1または-1=最新。
     * @return 引数の履歴
     */
    const element_type &at(int index) const
    {
        if (index < 0)
            return history.at(history.size() + index);
        else
            return history.at(index);
    }
    /**
     * @brief 添字アクセスのオーバーロード
     * @param index インデックス。0=最古、size()-1または-1=最新。
     * @return 引数の履歴
     */
    inline const element_type &operator[](int index) const { return at(index); }
    /**
     * @brief 要素を追加する
     * @param args 引数
     */
    inline void push_back(const element_type &args) { history.push_back(args); }

private:
    /// 引数の履歴
    std::vector<element_type> history;
};

/**
 * @brief void型用の特殊化
 */
template <>
struct _ArgumentsHistory<void>
{
    using element_type = void;
};
} // namespace Test

#endif // MOCK_HELPER_H
