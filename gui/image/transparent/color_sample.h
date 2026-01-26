#ifndef COLOR_SAMPLE_H
#define COLOR_SAMPLE_H

#include <QFrame>

#ifdef _TEST_ColorSample
namespace Test {
class TestColorSample;
}
#endif

/**
 * @brief 色付きの四角を表示するだけのウィジェット
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
    ~ColorSample() override = default;

    /**
     * @brief 色を設定し、UIを更新する
     * @param newColor 設定する色
     */
    void setColor(const QColor &newColor);
    /**
     * @brief 現在設定されている色を返す
     * @return 現在設定されている色
     */
    [[nodiscard]] const QColor &color() const { return _color; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    /// 表示する色
    QColor _color;

    /**
     * @brief 線の幅を除いた領域を返す
     * @return 線の幅を除いた領域
     */
    [[nodiscard]] QRect contentRect() const
    {
        return frameRect().marginsRemoved(QMargins(1, 1, 1, 1) * (lineWidth() + midLineWidth()));
    }

#ifdef _TEST_ColorSample
    friend class Test::TestColorSample;
#endif
};

#endif // COLOR_SAMPLE_H
