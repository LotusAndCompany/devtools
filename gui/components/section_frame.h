#ifndef GUI_COMPONENTS_SECTION_FRAME_H
#define GUI_COMPONENTS_SECTION_FRAME_H

#include <QFrame>

class QCheckBox;
class QVBoxLayout;

/**
 * @brief 折りたたみ可能なセクションフレーム
 *
 * QCheckBox トグルで body の表示/非表示を切り替える。
 * ImageToolsUnifiedGUI の resizeSection/transformSection
 * 等のパターンを汎用化した共有コンポーネント。
 */
class SectionFrame : public QFrame
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param title セクションのタイトル（トグルラベルとして表示される）
     * @param parent 親ウィジェット
     */
    explicit SectionFrame(const QString &title, QWidget *parent = nullptr);

    /**
     * @brief セクションのボディウィジェットを設定する
     * @param body ボディとして表示するウィジェット（所有権はこのフレームに移る）
     */
    void setBody(QWidget *body);

    /**
     * @brief 現在のボディウィジェットを取得する
     * @return 設定されているボディウィジェット。未設定なら nullptr
     */
    [[nodiscard]] QWidget *body() const;

    /**
     * @brief セクションが展開されているかを返す
     * @return 展開されていれば true
     */
    [[nodiscard]] bool isExpanded() const;

public slots:
    /**
     * @brief セクションの展開状態を設定する
     * @param expanded true で展開、false で折りたたみ
     */
    void setExpanded(bool expanded);

signals:
    /**
     * @brief 展開状態が変化したときに発せられる
     * @param expanded 新しい展開状態
     */
    void expandedChanged(bool expanded);

private:
    void updateToggleText(bool expanded);

    QCheckBox *m_toggle;
    QWidget *m_body;
    QVBoxLayout *m_layout;
    QString m_title;
};

#endif // GUI_COMPONENTS_SECTION_FRAME_H
