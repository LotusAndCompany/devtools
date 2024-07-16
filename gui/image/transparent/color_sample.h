#ifndef COLOR_SAMPLE_H
#define COLOR_SAMPLE_H

#include <QFrame>

/**
 * @brief 色付きの四角を表示するだけのウィジェット
 * @details クリックすると `QColorDialog` が開き、色を変更できる
 * @todo 必要があれば `QColorDialog` の色のセットを保存/読み込みする機能を追加する
 */
class ColorSample : public QFrame
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit ColorSample(QWidget *parent = nullptr);
    ~ColorSample() = default;

    /**
     * @brief 色を設定し、UIを更新する
     * @param newColor 設定する色
     */
    void setColor(const QColor &newColor);
    /**
     * @brief 現在設定されている色を返す
     * @return 現在設定されている色
     */
    const QColor &color() const { return _color; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    /**
     * @brief 色が変更された時に発せられる
     * @param color 変更後の色
     */
    void colorChanged(const QColor color);

private:
    /// 表示する色
    QColor _color;

    /**
     * @brief 線の幅を除いた領域を返す
     * @return 線の幅を除いた領域
     */
    inline QRect contentRect() const
    {
        return frameRect().marginsRemoved(QMargins(1, 1, 1, 1) * (lineWidth() + midLineWidth()));
    }

private slots:
    /**
     * @brief カラーピッカーで色が選択された時の処理
     * @param newColor 選択された色
     */
    void onColorSelected(const QColor &newColor);
};

#endif // COLOR_SAMPLE_H
