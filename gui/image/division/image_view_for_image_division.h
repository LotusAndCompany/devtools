#ifndef IMAGE_VIEW_FOR_IMAGE_DIVISION_H
#define IMAGE_VIEW_FOR_IMAGE_DIVISION_H

#include "../basic/image_view.h"

#include <QLabel>

namespace Ui {
class BasicImageView;
}

#ifdef _TEST_ImageViewForImageDivision
namespace Test {
class TestImageViewForImageDivision;
}
#endif

namespace ImageViewForImageDivisionInternal {
class LabelWithGrid;
}

/**
 * @brief 画像分割ツール用のImageView。画像にグリッドを表示できる。
 */
class ImageViewForImageDivision : public BasicImageView
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    ImageViewForImageDivision(QWidget *parent = nullptr);

    /**
     * @brief グリッドの大きさを設定する。大きさ0のグリッドを設定すると非表示になる。
     * @param newGridSize グリッドの大きさ
     */
    void setGridSize(const QSizeF &newGridSize);

    void updateScale(double newScale) override;

private:
    /**
     * @brief ui->imageを返す
     * @return ui->image
     */
    [[nodiscard]] ImageViewForImageDivisionInternal::LabelWithGrid *ui_image() const;

    /// 拡大率を適用していないグリッドの大きさ
    QSizeF gridSize;

#ifdef _TEST_ImageViewForImageDivision
    friend class Test::TestImageViewForImageDivision;
#endif
};

namespace ImageViewForImageDivisionInternal {
/**
 * @brief グリッドを表示できるQLabel
 * @todo グレーに塗りつぶせるようにする
 */
class LabelWithGrid : public QLabel
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    LabelWithGrid(QWidget *parent = nullptr) : QLabel(parent) {}
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief グリッドの大きさを設定する。大きさ0のグリッドを設定すると非表示になる。
     * @param scaledGridSize グリッドの大きさ(拡大済み)
     */
    void setGridSize(const QSizeF &scaledGridSize);

private:
    /// 実際に描画されるグリッドの大きさ
    QSizeF gridSize;

#ifdef _TEST_ImageViewForImageDivision
    friend class Test::TestImageViewForImageDivision;
#endif
};
} // namespace ImageViewForImageDivisionInternal

#endif // IMAGE_VIEW_FOR_IMAGE_DIVISION_H
