#ifndef IMAGE_VIEW_FOR_IMAGE_TRANSPARENT_H
#define IMAGE_VIEW_FOR_IMAGE_TRANSPARENT_H

#include "../basic/image_view.h"

#include <QLabel>

/*
namespace Ui {
class BasicImageView;
}
*/

#ifdef _TEST_ImageViewForImageTransparent
#include "gui/image/basic/ui_image_view.h"

namespace Test {
class TestImageViewForImageTransparent;
}
#endif

namespace _ImageViewForImageTransparentInternal {
class ClickableLabel;
}

/**
 * @brief 画像透明化ツール用のImageView。クリックした場所と色とを取得できる。
 * @todo 背景を市松模様にする
 */
class ImageViewForImageTransparent : public BasicImageView
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    ImageViewForImageTransparent(QWidget *parent = nullptr);

signals:
    /**
     * @brief ピクセルが選択された時に発せられるシグナル
     * @param point 選択した場所
     * @param color 選択場所の色
     */
    void pixelSelected(const QPoint &point, const QColor &color);

private slots:
    /**
     * @brief 画像がクリックされた時の処理
     * @param point クリックされた場所
     */
    void onLabelClicked(const QPoint &point);

private:
    /**
     * @brief ui->imageを返す
     * @return ui->image
     */
    _ImageViewForImageTransparentInternal::ClickableLabel *ui_image() const;

#ifdef _TEST_ImageViewForImageTransparent
    friend class Test::TestImageViewForImageTransparent;
#endif
};

namespace _ImageViewForImageTransparentInternal {
/**
 * @brief クリックしたピクセルの座標と色とを取得できるラベル
 */
class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    ClickableLabel(QWidget *parent = nullptr)
        : QLabel(parent)
    {}

signals:
    /**
     * @brief 左クリックされた時に発せられるシグナル
     * @param point 左クリックした場所
     */
    void clicked(const QPoint &point);

protected:
    void mousePressEvent(QMouseEvent *) override;
};
} // namespace _ImageViewForImageTransparentInternal

#endif // IMAGE_VIEW_FOR_IMAGE_TRANSPARENT_H
