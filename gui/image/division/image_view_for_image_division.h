#ifndef IMAGE_VIEW_FOR_IMAGE_DIVISION_H
#define IMAGE_VIEW_FOR_IMAGE_DIVISION_H

#include "../basic/image_view.h"

#include <QLabel>

namespace Ui {
class BasicImageView;
}

#ifdef _TEST_ImageViewForImageDivision
#include "gui/image/basic/ui_image_view.h"

namespace Test {
class TestImageViewForImageDivision;
}
#endif

namespace _ImageViewForImageDivisionInternal {
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
    void setGridSize(const QSize &newGridSize);

private:
    /**
     * @brief ui->imageを返す
     * @return ui->image
     */
    _ImageViewForImageDivisionInternal::LabelWithGrid *ui_image() const;

#ifdef _TEST_ImageViewForImageDivision
    friend class Test::TestImageViewForImageDivision;
#endif
};

namespace _ImageViewForImageDivisionInternal {
/**
 * @brief グリッドを表示できるQLabel
 */
class LabelWithGrid : public QLabel
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    LabelWithGrid(QWidget *parent = nullptr)
        : QLabel(parent)
    {}
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief グリッドの大きさを設定する。大きさ0のグリッドを設定すると非表示になる。
     * @param scaledGridSize グリッドの大きさ(拡大済み)
     */
    void setGridSize(const QSize &scaledGridSize);

private:
    /// グリッドの大きさ
    QSize gridSize;
};
} // namespace _ImageViewForImageDivisionInternal

#endif // IMAGE_VIEW_FOR_IMAGE_DIVISION_H
