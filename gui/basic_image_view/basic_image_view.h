#ifndef BASIC_IMAGE_VIEW_H
#define BASIC_IMAGE_VIEW_H

#include <QPixmap>
#include <QWidget>

namespace Ui {
class BasicImageView;
}

/**
 * @brief 画像編集系のGUIツールで画像を表示する部分
 */
class BasicImageView : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param 親ウィジェット
     */
    explicit BasicImageView(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~BasicImageView();

    /**
     * @brief 画像データを設定する。空の画像データを指定すると "No Image" になる
     * @param pixmap 画像データ
     * @param reset `true`なら拡大率をリセットする
     */
    void setPixmap(const QPixmap &pixmap, bool reset = false);

private slots:
    /**
     * @brief 拡大ボタンが押された時の処理
     */
    void onZoomInButtonPressed();
    /**
     * @brief 縮小ボタンが押された時の処理
     */
    void onZoomOutButtonPressed();

private:
    /// 拡大率の最小値
    static constexpr const double minScale = 0.1;
    /// 拡大率の最大値
    static constexpr const double maxScale = 10.0;
    /**
     * @brief zoomIn() zoomOut() で変化させる拡大率の基底
     * @details わざわざ対数を使うのは、例えばx0.1↔︎x1.0、x1.0↔︎x10がそれぞれ同じ回数の操作で変更できる方が自然だから。@n
     *          変更できるようにはなっているが、基本的には変更しない想定。
     */
    static constexpr const double scaleBase = 10.0;
    /// 一回の zoomIn() zoomOut() で変化する拡大率(対数スケール)
    static constexpr const double zoomStep = 0.2;

    Ui::BasicImageView *const ui;

    /// 実際の拡大率
    double scale;
    /// 拡大縮小前の画像データ。 setPixmap() によってのみ変更される
    QPixmap original;

    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    /**
     * @brief 拡大率を@f$ scaleBase^{zoomStep} @f$だけ上げる。 scale が maxScale と等しければ何もしない。
     */
    void zoomIn();
    /**
     * @brief 拡大率を@f$ scaleBase^{zoomStep} @f$だけ下げる。 scale が minScale と等しければ何もしない。
     */
    void zoomOut();
    /**
     * @brief scale を変更し、UIに反映する
     * @param newScale 新しい scale の値
     */
    void updateScale(double newScale); // NOTE: 関数名を改めた方が良いかも
};

#endif // BASIC_IMAGE_VIEW_H
