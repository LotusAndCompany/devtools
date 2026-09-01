#ifndef COLOR_CONVERTER_H
#define COLOR_CONVERTER_H

#include <QColor>
#include <QObject>
#include <QVector>

/**
 * @brief カラー変換機能のロジック部分
 * @details RGB/HEX/HSL/HSV/CMYK 間の相互変換、2色間の色差(ΔE)・コントラスト比の計算、
 *          現在色の履歴管理、比較用の「From」「To」カラーの管理を行う。
 *          GUIからは ColorConverterGUI を通じて利用される想定。
 *
 * @sa ColorConverterGUI
 */
class ColorConverter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief コントラスト比の判定結果
     */
    enum class ContrastLevel {
        Fail,   ///< WCAG基準を満たさない
        AA_Large, ///< AA(大きい文字向け)を満たす
        AA,     ///< AAを満たす
        AAA,    ///< AAAを満たす
    };

    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit ColorConverter(QObject *parent = nullptr);
    ~ColorConverter() override = default;

    ColorConverter(const ColorConverter &) = delete;
    ColorConverter &operator=(const ColorConverter &) = delete;
    ColorConverter(ColorConverter &&) = delete;
    ColorConverter &operator=(ColorConverter &&) = delete;

    /// 履歴として保持する最大件数
    static constexpr int HISTORY_MAX_SIZE = 12;

    // ------------------------------------------------------------------
    // 静的な変換ユーティリティ
    // ------------------------------------------------------------------

    /**
     * @brief QColorをHEX文字列に変換する (例: "#3B82F6")
     * @param color 変換対象の色
     * @param includeAlpha アルファ値を含める場合はtrue (例: "#3B82F6FF")
     * @return HEX文字列(大文字)
     */
    static QString toHex(const QColor &color, bool includeAlpha = false);

    /**
     * @brief HEX文字列からQColorへ変換する
     * @param hex "#RGB" "#RRGGBB" "#RRGGBBAA" のいずれかの形式
     * @param ok 変換に成功したかどうかを格納する(nullptr可)
     * @return 変換されたQColor。失敗時は無効なQColor
     */
    static QColor fromHex(const QString &hex, bool *ok = nullptr);

    /**
     * @brief "rgb(r, g, b)" / "rgba(r, g, b, a)" 形式の文字列に変換する
     */
    static QString toRgbString(const QColor &color);

    /**
     * @brief "rgb(...)" / "rgba(...)" 形式の文字列からQColorへ変換する
     */
    static QColor fromRgbString(const QString &text, bool *ok = nullptr);

    /**
     * @brief "hsl(h, s%, l%)" 形式の文字列に変換する
     */
    static QString toHslString(const QColor &color);

    /**
     * @brief "hsl(...)" 形式の文字列からQColorへ変換する
     */
    static QColor fromHslString(const QString &text, bool *ok = nullptr);

    /**
     * @brief "hsv(h, s%, v%)" 形式の文字列に変換する
     */
    static QString toHsvString(const QColor &color);

    /**
     * @brief "hsv(...)" 形式の文字列からQColorへ変換する
     */
    static QColor fromHsvString(const QString &text, bool *ok = nullptr);

    /**
     * @brief "cmyk(c%, m%, y%, k%)" 形式の文字列に変換する
     */
    static QString toCmykString(const QColor &color);

    /**
     * @brief "cmyk(...)" 形式の文字列からQColorへ変換する
     */
    static QColor fromCmykString(const QString &text, bool *ok = nullptr);

    /**
     * @brief CIE76によるΔE(知覚的な色差)を計算する
     * @details 0に近いほど2色は近似している。一般に 2.3 以上で人間が知覚できる差とされる。
     * @return ΔE値(0以上)
     */
    static double deltaE76(const QColor &lhs, const QColor &rhs);

    /**
     * @brief WCAG 2.x に基づく相対輝度を計算する
     * @return 0.0(黒)〜1.0(白)の相対輝度
     */
    static double relativeLuminance(const QColor &color);

    /**
     * @brief WCAG 2.x に基づくコントラスト比を計算する
     * @return 1.0〜21.0の範囲のコントラスト比
     */
    static double contrastRatio(const QColor &lhs, const QColor &rhs);

    /**
     * @brief コントラスト比からWCAG適合レベルを判定する(通常サイズの文字を想定)
     */
    static ContrastLevel contrastLevel(double ratio);

    /**
     * @brief WCAG適合レベルを短いラベル文字列にする (例: "AAA", "AA", "Fail")
     */
    static QString contrastLevelLabel(ContrastLevel level);

    // ------------------------------------------------------------------
    // インスタンスの状態(現在色・比較・履歴)
    // ------------------------------------------------------------------

    /**
     * @brief 現在編集中の色を返す
     */
    [[nodiscard]] QColor currentColor() const { return m_currentColor; }

    /**
     * @brief 現在編集中の色を設定する
     * @details 「To」カラーも同時に更新され、履歴に追加される。
     * @param color 新しい色
     */
    void setCurrentColor(const QColor &color);

    /**
     * @brief 比較用の「From」カラーを返す
     */
    [[nodiscard]] QColor fromColor() const { return m_fromColor; }

    /**
     * @brief 比較用の「To」カラーを返す
     * @details 現在は常に currentColor() と同じ値になる。
     */
    [[nodiscard]] QColor toColor() const { return m_toColor; }

    /**
     * @brief 現在の色を「From」として固定する(「Fromに固定」ボタンに対応)
     */
    void lockCurrentAsFrom();

    /**
     * @brief 「From」と「To」を入れ替える(「入れ替え」ボタンに対応)
     * @details 入れ替え後、現在色は新しい「To」の値になる。
     */
    void swapFromTo();

    /**
     * @brief 現在の「From」「To」間のΔEを返す
     */
    [[nodiscard]] double comparisonDeltaE() const { return deltaE76(m_fromColor, m_toColor); }

    /**
     * @brief 現在の「From」「To」間のコントラスト比を返す
     */
    [[nodiscard]] double comparisonContrastRatio() const
    {
        return contrastRatio(m_fromColor, m_toColor);
    }

    /**
     * @brief 色の履歴を返す(新しい順)
     */
    [[nodiscard]] const QVector<QColor> &history() const { return m_history; }

    /**
     * @brief 履歴をクリアする
     */
    void clearHistory();

signals:
    /**
     * @brief 現在色が変化した時に送出される
     * @param color 新しい現在色
     */
    void currentColorChanged(const QColor &color);

    /**
     * @brief 比較用の「From」「To」が変化した時に送出される
     */
    void comparisonChanged();

    /**
     * @brief 履歴が変化した時に送出される
     */
    void historyChanged();

private:
    /// 現在編集中の色
    QColor m_currentColor{"#3B82F6"};
    /// 比較用の基準色(固定)
    QColor m_fromColor{"#3B82F6"};
    /// 比較用の対象色(currentColorと連動)
    QColor m_toColor{"#3B82F6"};
    /// 色の履歴(先頭が最新)
    QVector<QColor> m_history;

    /**
     * @brief 履歴に色を追加する(重複は先頭に移動、上限を超えたら古いものを削除)
     * @param color 追加する色
     */
    void addToHistory(const QColor &color);
};

#endif // COLOR_CONVERTER_H
