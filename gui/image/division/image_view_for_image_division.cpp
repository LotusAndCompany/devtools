#include "image_view_for_image_division.h"

#include <QPainter>
#include "gui/image/basic/ui_image_view.h"

using namespace _ImageViewForImageDivisionInternal;

ImageViewForImageDivision::ImageViewForImageDivision(QWidget *parent)
    : BasicImageView(parent)
{
    // ui->imageを置き換える
    delete ui->image;
    ui->image = new LabelWithGrid(this);
    ui->scrollArea->setWidget(ui->image);
}

LabelWithGrid *ImageViewForImageDivision::ui_image() const
{
    return static_cast<LabelWithGrid *>(ui->image);
}

void ImageViewForImageDivision::setGridSize(const QSize &newGridSize)
{
    ui_image()->setGridSize(newGridSize.scaled(QSize(scale, scale), Qt::KeepAspectRatio));
}

void LabelWithGrid::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    if (gridSize.isEmpty() || pixmap().isNull())
        return;

    QPainter painter(this);

    // NOTE: 白とのXORを取る事で反対色の線を描画する
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setPen(Qt::white);

    const int xMax = pixmap().size().width();
    const int yMax = pixmap().size().height();

    for (int x = gridSize.width(); x < xMax; x += gridSize.width())
        painter.drawLine(QPoint(x, 0), QPoint(x, yMax));
    for (int y = gridSize.height(); y < yMax; y += gridSize.height())
        painter.drawLine(QPoint(0, y), QPoint(xMax, y));
}

void LabelWithGrid::setGridSize(const QSize &scaledGridSize)
{
    gridSize = scaledGridSize;
    update();
}
