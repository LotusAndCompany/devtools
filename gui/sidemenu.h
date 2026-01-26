#ifndef SIDEMENU_H
#define SIDEMENU_H

#include "core/tool/tool_id_fields.h"

#include <QMap>
#include <QWidget>

class QButtonGroup;
class QAbstractButton;
class SidemenuItem;

namespace Ui {
class Sidemenu;
}

/**
 * @brief サイドメニュー
 */
class Sidemenu : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit Sidemenu(QWidget *parent = nullptr) noexcept(false);
    /**
     * デストラクタ
     */
    ~Sidemenu() override;

    /**
     * @brief サイドメニューのID
     * @details ID::MIN, ID::WELCOME, ID::MAX 以外は TOOL_ID_FIELDS() を展開する@n
     *          ID::WELCOME, ID::MAX 以外はTool::IDと一致する
     *
     * @sa Tool::ID
     */
    enum class ID {
        MIN,              ///< 最小値
        TOOL_ID_FIELDS(), // ここに展開する
        WELCOME,          ///< ようこそ画面
        MAX,              ///< 最大値
    };
    /// 比較用のIDの最小値
    constexpr static const int ID_MIN = static_cast<int>(ID::MIN);
    /// 比較用のIDの最大値
    constexpr static const int ID_MAX = static_cast<int>(ID::MAX);

    /**
     * @brief 渡された ID が正しいか判定する
     * @details 原則として正しいIDが渡される想定で、万が一間違った ID
     * が渡された場合には例外を出す@n
     *          @b 例外が発生する場合はコードの修正が必要
     * @param id 対象の ID
     * @exception InvalidArgumentException &lt;int&gt; 間違った ID が渡された場合
     */
    static void validateID(ID id) noexcept(false);
    /**
     * @brief 渡された ID に対応するアイコンを返す
     * @param id
     * @return アイコン
     * @exception InvalidArgumentException &lt;int&gt; 間違った ID が渡された場合
     */
    static QIcon icon(ID id) noexcept(false);

    /**
     * @brief 指定したIDのメニューを選択状態にする
     * @param id 対象ID
     */
    void selectItem(ID id);

signals:
    /**
     * @brief サイドメニューのボタンが押された時に送られる
     * @param id 押されたサイドメニューID
     * @details ContentsArea::onSidemenuItemChanged(Sidemenu::ID id) と接続される
     *
     * @sa MainWindow::MainWindow(QWidget *parent)
     */
    void itemSelected(ID id);

private:
    /// 例外に設定するメッセージ
    static const QString invalidSidemenuIDReason;

    /// UI
    Ui::Sidemenu *const ui;

    /// サイドメニューの各ボタンを含むボタングループ(排他)
    QButtonGroup *const buttonGroup;

    /// 全てのサイドメニューアイテム
    QList<SidemenuItem *> allItems;

    /**
     * @brief 変更系のイベントを処理する
     * @param event 処理するイベント
     * @details 言語の変更をUIに反映している
     */
    void changeEvent(QEvent *event) override;
    /**
     * @brief ボタンを追加する
     * @param id 追加するボタンのID
     * @exception InvalidArgumentException &lt;int&gt; 間違った ID が渡された場合
     */
    void registerItem(ID id) noexcept(false);
    /**
     * @brief 検索テキストに基づいてアイテムの表示/非表示を制御する
     * @param searchText 検索テキスト
     */
    void filterItems(const QString &searchText);

    /**
* @brief サイドメニューのボタンが切り替わ �合

   * @param id 切り替わった� ���ID
* @param checked ボタ � �
        */
    void onButtonToggled(int id, bool checked);
    /**
     * @brief 検索ボックスのテキストが変更された時の処理
     * @param text 変更後のテキスト
     */
    void onSearchTextChanged(const QString &text);
};

#endif // SIDEMENU_H
