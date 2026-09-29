#ifndef TIMESTAMP_CONVERSION_GUI_H
#define TIMESTAMP_CONVERSION_GUI_H

#include "features/framework/gui/gui_tool.h"

#include <QDateTime>
#include <QWidget>

class QDateTimeEdit;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;

/**
 * @brief UNIXタイムスタンプ・日時・ISO8601形式を相互変換するツールのGUI
 * @details 各行が同じ瞬間を異なる表現で表示する。いずれかの行の変換ボタンを押すと、
 *          その値を基準時刻として他の全ての行に反映する。
 */
class TimestampConversionGUI : public GuiTool
{
    Q_OBJECT

public:
    explicit TimestampConversionGUI(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi();
    void retranslateUi();

    /**
     * @brief グリッドに1行分(ラベル・入力欄・変換ボタン・コピーボタン)を追加する
     * @param grid 追加先のグリッドレイアウト
     * @param row 追加する行番号
     * @param label 生成したラベルを受け取る出力先
     * @param editor 入力欄ウィジェット(呼び出し側で生成済み)
     * @param convertButton 生成した変換ボタンを受け取る出力先
     * @param copyButton 生成したコピーボタンを受け取る出力先
     */
    void addRow(QGridLayout *grid, int row, QLabel *&label, QWidget *editor,
                QPushButton *&convertButton, QPushButton *&copyButton);

    /**
     * @brief 基準時刻(UTC)を全ての表示欄に反映する
     * @details utcInstantが不正な場合はエラーメッセージを表示するのみで、表示欄は更新しない
     * @param utcInstant 反映する基準時刻(UTC)
     */
    void applyCanonical(const QDateTime &utcInstant);

    QLabel *secondsLabel{nullptr};
    QLineEdit *secondsEdit{nullptr};
    QPushButton *secondsConvertButton{nullptr};
    QPushButton *secondsCopyButton{nullptr};

    QLabel *millisecondsLabel{nullptr};
    QLineEdit *millisecondsEdit{nullptr};
    QPushButton *millisecondsConvertButton{nullptr};
    QPushButton *millisecondsCopyButton{nullptr};

    QLabel *localLabel{nullptr};
    QDateTimeEdit *localEdit{nullptr};
    QPushButton *localConvertButton{nullptr};
    QPushButton *localCopyButton{nullptr};

    QLabel *utcLabel{nullptr};
    QDateTimeEdit *utcEdit{nullptr};
    QPushButton *utcConvertButton{nullptr};
    QPushButton *utcCopyButton{nullptr};

    QLabel *isoUtcLabel{nullptr};
    QLineEdit *isoUtcEdit{nullptr};
    QPushButton *isoUtcConvertButton{nullptr};
    QPushButton *isoUtcCopyButton{nullptr};

    QLabel *isoLocalLabel{nullptr};
    QLineEdit *isoLocalEdit{nullptr};
    QPushButton *isoLocalConvertButton{nullptr};
    QPushButton *isoLocalCopyButton{nullptr};

    /// 現在時刻を全行に設定するボタン
    QPushButton *nowButton{nullptr};
    /// エラーメッセージ表示用ラベル
    QLabel *statusLabel{nullptr};
};

#endif // TIMESTAMP_CONVERSION_GUI_H
