#ifndef TOOL_H
#define TOOL_H

#include "tool_id_fields.h"

#include <QObject>

#ifdef _TEST_Tool
namespace Test {
class TestTool;
}
#endif

/**
 * @brief 各ツールのロジック部分の基底クラス
 */
class Tool : public QObject
{
public:
    Tool() = delete;
    Tool(const Tool &) = delete;
    Tool(Tool &&) = delete;
    Tool &operator=(const Tool &) = delete;
    Tool &operator=(Tool &&) = delete;
    /**
     * @brief デストラクタ
     */
    ~Tool() override = default;

    /**
     * @brief ツールのID
     * @details ID::MIN と ID::MAX 以外は TOOL_ID_FIELDS() を展開する@n
     *          IDの数値はビルド間で変わるため、永続化には #stringID を使う
     */
    enum class ID : uint8_t {
        MIN,              ///< 最小値
        TOOL_ID_FIELDS(), // ここに展開する
        MAX,              ///< 最大値
    };
    /**
     * @brief ツールの文字列ID 各ツールに分かりやすい一意な文字列を設定する
     */
    const QString stringID; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

    /// 比較用のIDの最小値
    constexpr static const int ID_MIN = static_cast<int>(ID::MIN);
    /// 比較用のIDの最大値
    constexpr static const int ID_MAX = static_cast<int>(ID::MAX);

protected:
    /**
     * @brief コンストラクタ
     * @details 派生クラスのコンストラクタから呼ばれる想定@n
     *          `id`が不正な場合は例外が発生する
     * @code
     * ExampleTool(Arg arg, QObject *parent)
     *      : Tool(ID::EXAMPLE_TOOL, "example-tool", parent)
     * {...}
     * @endcode
     * @param id ツールID
     * @param stringID ツールの文字列ID
     * @param parent 親オブジェクト
     */
    explicit Tool(ID id, QString stringID, QObject *parent = nullptr) noexcept(false);

    /// ツールIDが無効
    static const QString &invalidToolIDReason();

    /**
     * @brief 渡された ID が正しいか判定する
     * @details 原則として正しいIDが渡される想定で、万が一間違った ID が渡された場合には例外を出す@n
     *          @b 例外が発生する場合はコードの修正が必要
     * @param id 対象の ID
     * @exception InvalidArgumentException &lt;int&gt; 間違った ID が渡された場合
     */
    static void validateID(ID id) noexcept(false);

private:
#ifdef _TEST_Tool
    friend class Test::TestTool;
#endif
};

#endif // TOOL_H
