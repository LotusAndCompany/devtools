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
 * @details 各行が同じ瞬間を異なる表現で表示する。いずれかの行で「Convert」を押すと、
 *          その値を基準時刻として他の全ての行に反映する。
 */
class TimestampConversionGUI : public GuiTool
{
    Q_OBJECT
    Q_DISABLE_COPY(TimestampConversionGUI)

public:
    explicit TimestampConversionGUI(QWidget *parent = nullptr);
    ~TimestampConversionGUI() override = default;
    TimestampConversionGUI(TimestampConversionGUI &&) = delete;
    TimestampConversionGUI &operator=(TimestampConversionGUI &&) = delete;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    /// 全ての行に現在時刻を設定する
    void onNowClicked();
    /// 秒単位のタイムスタンプ行を基準に他の行を更新する
    void onConvertFromSecondsClicked();
    /// ミリ秒単位のタイムスタンプ行を基準に他の行を更新する
    void onConvertFromMillisecondsClicked();
    /// ローカル日時行を基準に他の行を更新する
    void onConvertFromLocalClicked();
    /// UTC日時行を基準に他の行を更新する
    void onConvertFromUtcClicked();
    /// ISO8601(UTC)行を基準に他の行を更新する
    void onConvertFromIsoUtcClicked();
    /// ISO8601(Local)行を基準に他の行を更新する
    void onConvertFromIsoLocalClicked();

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
     * @brief 変換に成功した基準時刻(UTC)を、指定した行以外の全ての表示欄に反映する
     * @param utcInstant 反映する基準時刻(UTC)
     */
    void applyCanonical(const QDateTime &utcInstant);
    /// エラーメッセージを表示する
    void showInvalidValueError();

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
