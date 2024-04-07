#ifndef TOOL_H
#define TOOL_H

#include <QObject>
#include "tool_id_fields.h"

/**
 * @brief 各ツールのロジック部分の基底クラス
 */
class Tool : public QObject
{
    Q_OBJECT

public:
    Tool() = delete;
    Tool(const Tool &) = delete;
    /**
     * @brief デストラクタ
     */
    virtual ~Tool() = default;

    /**
     * @brief ツールのID
     * @details ID::MIN と ID::MAX 以外は TOOL_ID_FIELDS() を展開する@n
     *          定義によって値が変わってしまうため、データの保存等に使う場合は #stringID を使う@n
     *          ID::MAX 以外は Sidemenu::IDと一致する
     * 
     * @sa Sidemenu::ID
     */
    enum class ID {
        MIN,              ///<最小値
        TOOL_ID_FIELDS(), // ここに展開する
        MAX,              ///<最大値
    };
    /**
     * @brief ツールの文字列ID 各ツールに分かりやすい一意な文字列を設定する
     */
    const QString stringID;

    /**
     * @brief 翻訳が必要な情報を格納する構造体
     */
    struct Translatable
    {
        /// ツールの名前
        QString name;
        // TODO: ツールの簡単な説明として表示できるようにする
        /// ツールの簡単な説明
        QString description;
    };

    /// 比較用のIDの最小値
    constexpr static const int ID_MIN = static_cast<int>(ID::MIN);
    /// 比較用のIDの最大値
    constexpr static const int ID_MAX = static_cast<int>(ID::MAX);

    /**
     * @brief 指定されたツールIDの情報(翻訳済み)を返す
     * @details 言語設定が変更された場合にこれを呼び出してUIに反映させる事を想定している
     * @param id ツールID
     * @return 指定されたツールIDの情報
     */
    static const Translatable translatable(ID id) noexcept(false);
    /**
     * @brief このインスタンスの情報を返す
     * @return このインスタンスの情報
     */
    const Translatable &translatable() const { return _translatable; }

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
    explicit Tool(ID id, const QString &stringID, QObject *parent = nullptr) noexcept(false);

    /// 例外に設定するメッセージ
    static const QString invalidToolIDReason;

    /**
     * @brief 渡された ID が正しいか判定する
     * @details 原則として正しいIDが渡される想定で、万が一間違った ID が渡された場合には例外を出す@n
     *          @b 例外が発生する場合はコードの修正が必要
     * @param id 対象の ID
     * @exception InvalidArgumentException &lt;int&gt; 間違った ID が渡された場合
     */
    static void validateID(ID id) noexcept(false);

    /**
     * @brief イベント処理
     * @details 言語が変更された場合に Translatable translatable(ID id) でテキストを更新する処理を実装している
     * @param event 発生したイベント
     * @return 処理した場合は`true`、無視した場合は`false`
     */
    virtual bool event(QEvent *event) override;

private:
    const ID id;
    Translatable _translatable;
};

#endif // TOOL_H
