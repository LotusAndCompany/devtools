#ifndef COLOR_SAMPLE_H
#define COLOR_SAMPLE_H

#include <QWidget>

/**
 * @brief 色付きの四角を表示するだけのウィジェット
 * @details クリックすると `QColorDialog` が開き、色を変更できる
 * @todo 必要があれば `QColorDialog` の色のセットを保存/読み込みする機能を追加する
 */
class ColorSample : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param initialColor 初期化に使う色
     * @param parent 親ウィジェット
     */
    explicit ColorSample(const QColor &initialColor, QWidget *parent = nullptr);
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

signals:
    /**
     * @brief 色が変更された時に発せられる
     * @param color 変更後の色
     */
    void colorChanged(const QColor color);

private:
    /// 表示する色
    QColor _color;
};

#endif // COLOR_SAMPLE_H
