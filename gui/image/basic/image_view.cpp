#include "image_view.h"

#include "ui_image_view.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QImageReader>
#include <QMimeData>
#include <QResizeEvent>
#include <QUrl>
#include <QWheelEvent>

#include <cmath>

namespace {
QString droppedImagePath(const QMimeData *mimeData)
{
    if (mimeData == nullptr || !mimeData->hasUrls()) {
        return {};
    }

    for (const QUrl &url : mimeData->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        const QString path = url.toLocalFile();
        if (!QFileInfo(path).isFile()) {
            continue;
        }

        if (!QImageReader::imageFormat(path).isEmpty()) {
            return path;
        }
    }

    return {};
}
} // namespace

// TODO: ScrollAreaをドラッグで操作できるようにする
BasicImageView::BasicImageView(QWidget *parent) : QWidget(parent), ui(new Ui::BasicImageView)
{
    ui->setupUi(this);
    ui->scalingUI->raise();
    setAcceptDrops(true);

    // NOTE: 少しだけ透明にする
    auto *opacityEffect = new QGraphicsOpacityEffect(ui->zoomInButton);
    opacityEffect->setOpacity(0.8);
    ui->zoomInButton->setGraphicsEffect(opacityEffect);

    opacityEffect = new QGraphicsOpacityEffect(ui->zoomOutButton);
    opacityEffect->setOpacity(0.8);
    ui->zoomOutButton->setGraphicsEffect(opacityEffect);

    connect(ui->zoomInButton, &QToolButton::clicked, this, &BasicImageView::onZoomInButtonPressed);
    connect(ui->zoomOutButton, &QToolButton::clicked, this,
            &BasicImageView::onZoomOutButtonPressed);
}

BasicImageView::~BasicImageView()
{
    delete ui;
}

void BasicImageView::setPixmap(const QPixmap &pixmap, bool reset)
{
    original = pixmap;

    if (reset) {
        updateScale(1.0);
    } else {
        updateScale(scale);
    }
}

void BasicImageView::resizeEvent(QResizeEvent *event)
{
    ui->scrollArea->resize(event->size());
    const auto s = event->size() - ui->scalingUI->size();
    ui->scalingUI->move(s.width(), s.height());
    event->accept();
}

void BasicImageView::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void BasicImageView::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
}

void BasicImageView::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);
}

void BasicImageView::dragEnterEvent(QDragEnterEvent *event)
{
    if (!droppedImagePath(event->mimeData()).isEmpty()) {
        event->acceptProposedAction();
        return;
    }

    QWidget::dragEnterEvent(event);
}

void BasicImageView::dropEvent(QDropEvent *event)
{
    const QString path = droppedImagePath(event->mimeData());
    if (path.isEmpty()) {
        QWidget::dropEvent(event);
        return;
    }

    emit loadFileSelected(path);
    event->acceptProposedAction();
}

void BasicImageView::onZoomInButtonPressed()
{
    if (!original.isNull()) {
        zoomIn();
    }
}

void BasicImageView::onZoomOutButtonPressed()
{
    if (!original.isNull()) {
        zoomOut();
    }
}

void BasicImageView::zoomIn()
{
    if (scale == maxScale) {
        return;
    }

    // NOTE: 底が2, e, 10の時はそれぞれlog2(), log(), log10()に置き換えられる
    const double logScale = log(scale) / log(scaleBase);
    // NOTE: 指数が2, eの時はそれぞれexp2(), exp()に置き換えられる
    const double nextScale = pow(scaleBase, logScale + zoomStep);

    updateScale(std::min(nextScale, maxScale));
}

void BasicImageView::zoomOut()
{
    if (scale == minScale) {
        return;
    }

    // NOTE: 底が2, e, 10の時はそれぞれlog2(), log(), log10()に置き換えられる
    const double logScale = log(scale) / log(scaleBase);
    // NOTE: 指数が2, eの時はそれぞれexp2(), exp()に置き換えられる
    const double nextScale = pow(scaleBase, logScale - zoomStep);

    updateScale(std::max(nextScale, minScale));
}

void BasicImageView::updateScale(double newScale)
{
    scale = newScale;

    ui->zoomOutButton->setDisabled(scale <= minScale);
    ui->zoomInButton->setDisabled(maxScale <= scale);

    // https://doc.qt.io/qt-6/qlocale.html#toString-8
    ui->scaleLabel->setText('x' + QString::number(scale, 'g', 2));

    if (!original.isNull()) {
        ui->image->setPixmap(original.scaled(original.size() * scale));
    } else {
        ui->image->setText(tr("No Image"));
    }
}
