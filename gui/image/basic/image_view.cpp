#include "image_view.h"
#include "ui_image_view.h"

#include <QDropEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <cmath>

BasicImageView::BasicImageView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BasicImageView)
{
    ui->setupUi(this);

    connect(ui->zoomInButton, &QToolButton::clicked, this, &BasicImageView::onZoomInButtonPressed);
    connect(ui->zoomOutButton, &QToolButton::clicked, this, &BasicImageView::onZoomOutButtonPressed);
}

BasicImageView::~BasicImageView()
{
    delete ui;
}

void BasicImageView::setPixmap(const QPixmap &pixmap, bool reset)
{
    original = pixmap;

    if (!original.isNull())
        ui->image->setText("");
    else
        ui->image->setText(tr("No Image"));

    if (reset)
        updateScale(1.0);
    else
        updateScale(scale);
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

void BasicImageView::dropEvent(QDropEvent *event)
{
    QWidget::dropEvent(event);
}

void BasicImageView::onZoomInButtonPressed()
{
    zoomIn();
}

void BasicImageView::onZoomOutButtonPressed()
{
    zoomOut();
}

void BasicImageView::zoomIn()
{
    if (scale == maxScale)
        return;

    // NOTE: 底が2, e, 10の時はそれぞれlog2(), log(), log10()に置き換えられる
    const double logScale = log(scale) / log(scaleBase);
    // NOTE: 指数が2, eの時はそれぞれexp2(), exp()に置き換えられる
    const double nextScale = pow(scaleBase, logScale + zoomStep);

    updateScale(std::min(nextScale, maxScale));
}

void BasicImageView::zoomOut()
{
    if (scale == minScale)
        return;

    // NOTE: 底が2, e, 10の時はそれぞれlog2(), log(), log10()に置き換えられる
    const double logScale = log(scale) / log(scaleBase);
    // NOTE: 指数が2, eの時はそれぞれexp2(), exp()に置き換えられる
    const double nextScale = pow(scaleBase, logScale - zoomStep);

    updateScale(std::max(nextScale, minScale));
}

void BasicImageView::updateScale(double newScale)
{
    scale = newScale;

    ui->zoomOutButton->setDisabled(scale == minScale);
    ui->zoomInButton->setDisabled(scale == maxScale);

    ui->scaleLabel->setText('x' + QString::number(scale, 'g', 2));

    // FIXME: 画像がnullだと"QPixmap::scaled: Pixmap is a null pixmap"が発生する
    ui->image->setPixmap(original.scaled(original.size() * scale));
}
