#include "image_view_for_image_transparent.h"

#include <QMouseEvent>
#include "gui/image/basic/ui_image_view.h"

using namespace _ImageViewForImageTransparentInternal;

ImageViewForImageTransparent::ImageViewForImageTransparent(QWidget *parent)
    : BasicImageView(parent)
{
    // NOTE: ui->imageを置き換える
    delete ui->image;
    ui->image = new ClickableLabel(this);
    ui->image->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    ui->scrollArea->setWidget(ui->image);
    connect(ui_image(),
            &ClickableLabel::clicked,
            this,
            &ImageViewForImageTransparent::onLabelClicked);

    BasicImageView::updateScale(1);
}

ClickableLabel *ImageViewForImageTransparent::ui_image() const
{
    return static_cast<ClickableLabel *>(ui->image);
}

void ImageViewForImageTransparent::onLabelClicked(const QPoint &point)
{
    if (point.x() < 0 || point.y() < 0)
        return;

    if (!ui_image()->pixmap().isNull()) {
        const auto pixel = QPoint(point.x() / scale, point.y() / scale);
        const QImage image = std::move(original.toImage());

        if (pixel.x() < image.width() && pixel.y() < image.height())
            emit pixelSelected(pixel, image.pixelColor(pixel));
    }
}

void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        emit clicked(event->pos());
    }
}
