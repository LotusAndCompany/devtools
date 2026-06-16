#ifndef GUI_COMPONENTS_STATUS_LABEL_H
#define GUI_COMPONENTS_STATUS_LABEL_H

#include <QLabel>

#include <cstdint>

/**
 * @brief エラー/警告/成功表示用ラベル
 *
 * `"color: red; font-size: 12px;"` の16回重複を置き換える共有コンポーネント。
 * enum class Level でスタイルを切り替える。
 */
class StatusLabel : public QLabel
{
    Q_OBJECT

public:
    /**
     * @brief メッセージの重要度レベル
     */
    enum class Level : std::uint8_t {
        Error,   ///< 赤色、エラー表示
        Warning, ///< オレンジ色、警告表示
        Success, ///< 緑色、成功表示
    };

    /**
     * @brief コンストラクタ（非表示状態で初期化される）
     * @param parent 親ウィジェット
     */
    explicit StatusLabel(QWidget *parent = nullptr);

    /**
     * @brief ラベルのスタイルレベルを設定する
     * @param level 設定するレベル
     */
    void setLevel(Level level);

    /**
     * @brief レベルとメッセージを同時に設定して表示する
     * @param level メッセージのレベル
     * @param message 表示するメッセージ
     */
    void showMessage(Level level, const QString &message);

    /**
     * @brief メッセージをクリアして非表示にする
     */
    void clear();

    /// エラースタイル定数
    static constexpr const char *kErrorStyle = "color: red; font-size: 12px;";
    /// 警告スタイル定数
    static constexpr const char *kWarningStyle = "color: orange; font-size: 12px;";
    /// 成功スタイル定数
    static constexpr const char *kSuccessStyle = "color: green; font-size: 12px;";

private:
    Level m_level;
    static const char *styleForLevel(Level level);
};

#endif // GUI_COMPONENTS_STATUS_LABEL_H
