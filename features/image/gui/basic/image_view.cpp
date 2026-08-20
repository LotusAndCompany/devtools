#include "image_view.h"

#include "features/framework/gui/design_system.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QImageReader>
#include <QLabel>
#include <QMimeData>
#include <QScrollArea>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
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

void buildScrollArea(Ui::BasicImageView *ui, QWidget *parent)
{
    ui->scrollArea = new QScrollArea(parent);
    DevTools::Ui::configureScrollView(ui->scrollArea);

    ui->scrollAreaWidgetContents = new QWidget();
    DevTools::Ui::configureExpandingSurface(ui->scrollAreaWidgetContents);

    auto *gridLayout = new QGridLayout(ui->scrollAreaWidgetContents);
    DevTools::Ui::applyFullBleedLayout(gridLayout);

    ui->image = new QLabel(BasicImageView::tr("No Image"), ui->scrollAreaWidgetContents);
    DevTools::Ui::configureImageSurface(ui->image);
    gridLayout->addWidget(ui->image, 0, 0);

    ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);
}

QToolButton *buildZoomButton(QWidget *parent, const QString &iconName, const QString &toolTip)
{
    auto *button = new QToolButton(parent);
    DevTools::Ui::configureZoomButton(button, iconName, toolTip);

    return button;
}

void buildScalingUI(Ui::BasicImageView *ui, QWidget *parent)
{
    ui->scalingUI = new QWidget(parent);

    auto *layout = new QVBoxLayout(ui->scalingUI);
    DevTools::Ui::applyPanelLayout(layout);

    ui->zoomInButton = buildZoomButton(ui->scalingUI, "zoom_in", BasicImageView::tr("Zoom In"));
    layout->addWidget(ui->zoomInButton);

    ui->zoomOutButton = buildZoomButton(ui->scalingUI, "zoom_out", BasicImageView::tr("Zoom Out"));
    layout->addWidget(ui->zoomOutButton);

    ui->scaleLabel = new QLabel("x1", ui->scalingUI);
    DevTools::Ui::configureCenteredLabel(ui->scaleLabel);
    layout->addWidget(ui->scaleLabel);
}
} // namespace

// TODO: ScrollAreaをドラッグで操作できるようにする
BasicImageView::BasicImageView(QWidget *parent) : QWidget(parent), ui(new Ui::BasicImageView)
{
    buildScrollArea(ui, this);
    buildScalingUI(ui, this);

    auto *layout = new QGridLayout(this);
    DevTools::Ui::applyFullBleedLayout(layout);
    layout->addWidget(ui->scrollArea, 0, 0);
    DevTools::Ui::addBottomRightOverlay(layout, ui->scalingUI);

    ui->scalingUI->raise();
    setAcceptDrops(true);

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
