#include "image_view_for_image_division.h"

#include "gui/image/basic/ui_image_view.h"

#include <QPainter>

using namespace ImageViewForImageDivisionInternal;

ImageViewForImageDivision::ImageViewForImageDivision(QWidget *parent) : BasicImageView(parent)
{
    // NOTE: ui->imageを置き換える
    delete ui->image;
    ui->image = new LabelWithGrid(this);
    ui->image->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    ui->scrollArea->setWidget(ui->image);
    BasicImageView::updateScale(1);
}

void ImageViewForImageDivision::updateScale(double newScale)
{
    BasicImageView::updateScale(newScale);
    setGridSize(gridSize);
    update();
}

LabelWithGrid *ImageViewForImageDivision::ui_image() const
{
    return dynamic_cast<LabelWithGrid *>(ui->image);
}

void ImageViewForImageDivision::setGridSize(const QSizeF &newGridSize)
{
    gridSize = newGridSize;
    ui_image()->setGridSize(newGridSize.scaled(scale * newGridSize, Qt::KeepAspectRatio));
}

void LabelWithGrid::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    if (gridSize.isEmpty() || pixmap().isNull()) {
        return;
    }

    QPainter painter(this);

    // NOTE: 白とのXORを取る事で反対色の線を描画する
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setPen(Qt::white);

    const int xMax = pixmap().size().width();
    const int yMax = pixmap().size().height();

    // NOTE: 座標系をpixmap(中央に描画される)に合わせる
    const auto topLeft = QPointF(width() - xMax, height() - yMax) / 2;
    painter.setTransform(QTransform::fromTranslate(topLeft.x(), topLeft.y()));

    /* NOTE: 警告が出ているが、gridSizeがxLimit,
     * yLimitに対して小さ過ぎたり丸め誤差が1に近いことは無い想定なので問題無いはず Variable 'x' with
     * floating point type 'double' should not be used as a loop counter
     * [clang-analyzer-security.FloatLoopCounter] Variable 'y' with floating point type 'double'
     * should not be used as a loop counter [clang-analyzer-security.FloatLoopCounter]
     */
    const double xLimit = xMax - 1;
    const double yLimit = yMax - 1;
    // NOLINTNEXTLINE(clang-analyzer-security.FloatLoopCounter)
    for (double x = gridSize.width(); x < xLimit; x += gridSize.width()) {
        painter.drawLine(QPointF(x, 0), QPointF(x, yMax));
    }
    // NOLINTNEXTLINE(clang-analyzer-security.FloatLoopCounter)
    for (double y = gridSize.height(); y < yLimit; y += gridSize.height()) {
        painter.drawLine(QPointF(0, y), QPointF(xMax, y));
    }
}

void LabelWithGrid::setGridSize(const QSizeF &scaledGridSize)
{
    gridSize = scaledGridSize;
    update();
}
