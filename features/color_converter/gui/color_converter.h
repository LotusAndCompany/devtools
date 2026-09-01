#ifndef COLOR_CONVERTER_GUI_H
#define COLOR_CONVERTER_GUI_H

#include "features/color_converter/core/color_converter.h"

#include <QColor>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;

/**
 * @brief 彩度(Saturation)・明度(Value)を2次元で選択するための矩形ピッカー
 * @details 選択中の色相(Hue)を基準にグラデーションを描画し、クリック/ドラッグで
 *          彩度・明度を選択する。値は0.0〜1.0で正規化されている。
 */
class SvPickerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SvPickerWidget(QWidget *parent = nullptr);

    /**
     * @brief 基準となる色相を設定する(グラデーションの再描画のみ、s/vは変化しない)
     * @param hue 色相 (0.0〜1.0)
     */
    void setHue(qreal hue);

    /**
     * @brief 選択位置を彩度・明度で設定する(シグナルは発火しない)
     */
    void setSaturationValue(qreal saturation, qreal value);

    [[nodiscard]] qreal saturation() const { return m_saturation; }
    [[nodiscard]] qreal value() const { return m_value; }

signals:
    /**
     * @brief ユーザー操作によって彩度・明度が変化した時に送出される
     */
    void saturationValueChanged(qreal saturation, qreal value);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    qreal m_hue{0.0};
    qreal m_saturation{1.0};
    qreal m_value{1.0};

    void updateFromPos(const QPoint &pos);
};

/**
 * @brief 色相・アルファ用の帯状グラデーションスライダー
 * @details 水平方向のバーにグラデーション(色相環 or 透明度)を描画し、
 *          クリック/ドラッグで値(0〜maximum)を選択する。
 */
class GradientSliderWidget : public QWidget
{
    Q_OBJECT

public:
    /// スライダーの種類。背景の描画方法が変わる。
    enum class Kind {
        Hue,   ///< 0〜359の色相環グラデーション
        Alpha, ///< 現在色を基準にした透明→不透明のグラデーション(市松模様付き)
    };

    explicit GradientSliderWidget(Kind kind, QWidget *parent = nullptr);

    /**
     * @brief アルファスライダーで使用する基準色(RGB成分)を設定する
     */
    void setBaseColor(const QColor &color);

    /**
     * @brief 値を設定する(シグナルは発火しない)
     * @param value Hueの場合0〜359、Alphaの場合0〜255
     */
    void setValue(int value);

    [[nodiscard]] int value() const { return m_value; }
    [[nodiscard]] int maximumValue() const;

signals:
    /**
     * @brief ユーザー操作によって値が変化した時に送出される
     */
    void valueChanged(int value);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    Kind m_kind;
    int m_value{0};
    QColor m_baseColor{Qt::red};

    void updateFromPos(const QPoint &pos);
};

/**
 * @brief カラー変換機能のメインGUI
 * @details Pencilデザイン「Color Converter」に基づく、SV/色相/アルファピッカー、
 *          HEX/RGB/HSL/HSV/CMYKの相互変換表示、履歴、From/To比較パネルを提供する。
 *          ロジックは ColorConverter に委譲する。
 *
 * @sa ColorConverter
 */
class ColorConverterGUI : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param converter カラー変換ロジック。本ウィジェットが所有権を持つ(親として設定される)
     * @param parent 親ウィジェット
     */
    explicit ColorConverterGUI(ColorConverter *converter, QWidget *parent = nullptr);
    ~ColorConverterGUI() override = default;

    ColorConverterGUI(const ColorConverterGUI &) = delete;
    ColorConverterGUI &operator=(const ColorConverterGUI &) = delete;
    ColorConverterGUI(ColorConverterGUI &&) = delete;
    ColorConverterGUI &operator=(ColorConverterGUI &&) = delete;

protected:
    /**
     * @brief スポイト機能でマウスをグラブしている間のクリックを処理する
     */
    void mousePressEvent(QMouseEvent *event) override;

private:
    /// カラー変換ロジック(本ウィジェットの子として所有される)
    ColorConverter *const m_converter;

    // --- ピッカーカード ---
    SvPickerWidget *m_svPicker{nullptr};
    GradientSliderWidget *m_hueSlider{nullptr};
    GradientSliderWidget *m_alphaSlider{nullptr};
    QWidget *m_previewSwatch{nullptr};
    QToolButton *m_eyedropperButton{nullptr};
    QWidget *m_historyRow{nullptr};
    QHBoxLayout *m_historyRowLayout{nullptr};

    // --- 値カード ---
    QLineEdit *m_hexInput{nullptr};
    QLineEdit *m_rgbInput{nullptr};
    QLineEdit *m_hslInput{nullptr};
    QLineEdit *m_hsvInput{nullptr};
    QLineEdit *m_cmykInput{nullptr};

    // --- 比較パネル ---
    QWidget *m_fromSwatch{nullptr};
    QWidget *m_toSwatch{nullptr};
    QLabel *m_deltaELabel{nullptr};
    QLabel *m_contrastLabel{nullptr};
    QLabel *m_contrastBadge{nullptr};
    QPushButton *m_lockFromButton{nullptr};
    QPushButton *m_swapButton{nullptr};

    /// スポイト実行中かどうか
    bool m_pickingFromScreen{false};

    QWidget *createPickerCard();
    QWidget *createValuesCard();
    void createValueRow(const QString &labelText, QLineEdit **inputOut, QGridLayout *grid,
                         int row, QWidget *rowParent);
    QWidget *createComparisonPanel();
    static QWidget *createSwatch(int size = 40);
    static void setSwatchColor(QWidget *swatch, const QColor &color);

    void rebuildHistoryRow();
    /// 現在色に合わせてピッカー・値カードの全表示を更新する
    void refreshFromColor(const QColor &color);
    void refreshComparison();

    void onSvChanged(qreal saturation, qreal value);
    void onHueChanged(int hue);
    void onAlphaChanged(int alpha);
    void onHexEdited();
    void onRgbEdited();
    void onHslEdited();
    void onHsvEdited();
    void onCmykEdited();
    void onCopyRequested(const QLineEdit *source);
    void onEyedropperClicked();
    void onHistorySwatchClicked(const QColor &color);
};

#endif // COLOR_CONVERTER_GUI_H
