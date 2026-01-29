#include "image_view_for_image_transparent.h"

#include "gui/image/basic/ui_image_view.h"

#include <QMouseEvent>

using namespace ImageViewForImageTransparentInternal;

ImageViewForImageTransparent::ImageViewForImageTransparent(QWidget *parent) : BasicImageView(parent)
{
    // NOTE: ui->imageを置き換える
    delete ui->image;
    ui->image = new ClickableLabel(this);
    ui->image->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    ui->scrollArea->setWidget(ui->image);
    connect(ui_image(), &ClickableLabel::clicked, this,
            &ImageViewForImageTransparent::onLabelClicked);

    BasicImageView::updateScale(1);
}

ClickableLabel *ImageViewForImageTransparent::ui_image() const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) -
    // コンストラクタで型が保証されている
    return static_cast<ClickableLabel *>(ui->image);
}

void ImageViewForImageTransparent::onLabelClicked(const QPoint &point)
{
    if (!ui_image()->pixmap().isNull()) {
        const qreal xOffset = (ui_image()->width() - ui_image()->pixmap().width()) / 2.0;
        const qreal yOffset = (ui_image()->height() - ui_image()->pixmap().height()) / 2.0;

        const auto pixel = QPoint(static_cast<int>((point.x() - xOffset) / scale),
                                  static_cast<int>((point.y() - yOffset) / scale));

        if (0 <= pixel.x() && 0 <= pixel.y() && pixel.x() < original.width() &&
            pixel.y() < original.height()) {
            const QImage image = original.toImage();
            emit pixelSelected(pixel, image.pixelColor(pixel));
        }
    }
}

void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        // NOTE: ウィジェットの左上を原点とした座標で、見た目の左上とは一致しない
        emit clicked(event->pos());
    }
}
