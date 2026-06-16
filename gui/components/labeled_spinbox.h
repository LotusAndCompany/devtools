#ifndef GUI_COMPONENTS_LABELED_SPINBOX_H
#define GUI_COMPONENTS_LABELED_SPINBOX_H

#include <QWidget>

class QLabel;
class QSpinBox;

/**
 * @brief ラベル + QSpinBox の水平レイアウト
 *
 * ImageToolsUnifiedGUI の widthValue/heightValue/hScaleValue/vScaleValue 等の
 * パターンを汎用化した共有コンポーネント。
 */
class LabeledSpinbox : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param label ラベルテキスト（tr() 済みの文字列を渡すこと）
     * @param parent 親ウィジェット
     */
    explicit LabeledSpinbox(const QString &label, QWidget *parent = nullptr);

    /**
     * @brief 値の範囲を設定する
     * @param min 最小値
     * @param max 最大値
     */
    void setRange(int min, int max);

    /**
     * @brief 現在の値を取得する
     * @return スピンボックスの現在値
     */
    [[nodiscard]] int value() const;

    /**
     * @brief 値を設定する
     * @param value 設定する値
     */
    void setValue(int value);

    /**
     * @brief 内部の QSpinBox を取得する
     * @return QSpinBox へのポインタ（追加設定用）
     */
    [[nodiscard]] QSpinBox *spinBox() const;

signals:
    /**
     * @brief 値が変更されたときに発せられる
     * @param value 新しい値
     */
    void valueChanged(int value);

private:
    QLabel *m_label;
    QSpinBox *m_spinBox;
};

#endif // GUI_COMPONENTS_LABELED_SPINBOX_H
