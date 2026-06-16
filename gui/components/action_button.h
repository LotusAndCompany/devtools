#ifndef GUI_COMPONENTS_ACTION_BUTTON_H
#define GUI_COMPONENTS_ACTION_BUTTON_H

#include <QPushButton>

/**
 * @brief アイコン + ラベルを持つアクションボタン
 *
 * data_conversion の pasteButton/clearButton/copyButton/saveButton や
 * image/basic/control の load/reset/save ボタンのパターンを汎用化した共有コンポーネント。
 * デフォルトのパディングスタイル ("padding: 8px;") が適用される。
 */
class ActionButton : public QPushButton
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param icon ボタンに表示するアイコン。空の QIcon でテキストのみのボタンになる
     * @param text ボタンに表示するテキスト（tr() 済みの文字列を渡すこと）
     * @param parent 親ウィジェット
     */
    ActionButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

    /**
     * @brief アイコンを設定する
     * @param icon 新しいアイコン
     */
    void setIcon(const QIcon &icon);

    /**
     * @brief テキストを設定する
     * @param text 新しいテキスト（tr() 済みの文字列を渡すこと）
     */
    void setText(const QString &text);

    /// 標準パディングスタイル定数
    static constexpr const char *kPaddingStyle = "padding: 8px;";
};

#endif // GUI_COMPONENTS_ACTION_BUTTON_H
