#include "image_view.h"

#include "features/framework/gui/icon_utils.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QIcon>
#include <QImageReader>
#include <QLabel>
#include <QMimeData>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSizePolicy>
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
    ui->scrollArea->setGeometry(0, 0, 400, 300);
    ui->scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->setAlignment(Qt::AlignCenter);

    ui->scrollAreaWidgetContents = new QWidget();
    ui->scrollAreaWidgetContents->setGeometry(0, 0, 400, 300);
    ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *gridLayout = new QGridLayout(ui->scrollAreaWidgetContents);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(0);

    ui->image = new QLabel(BasicImageView::tr("No Image"), ui->scrollAreaWidgetContents);
    ui->image->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->image->setAlignment(Qt::AlignCenter);
    gridLayout->addWidget(ui->image, 0, 0);

    ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);
}

QToolButton *buildZoomButton(QWidget *parent, const QString &iconName, const QString &toolTip)
{
    auto *button = new QToolButton(parent);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->setMinimumSize(32, 32);
    button->setMaximumSize(32, 32);
    button->setToolTip(toolTip);
    button->setAutoFillBackground(true);

    const QIcon icon = IconUtils::themedIcon(iconName);
    if (icon.isNull()) {
        // NOTE: テーマアイコンが解決できない環境では、ボタンが空にならないよう toolTip を表示する
        button->setText(toolTip);
    } else {
        button->setIcon(icon);
        button->setIconSize(QSize(24, 24));
    }

    // NOTE: 少しだけ透明にする
    auto *opacityEffect = new QGraphicsOpacityEffect(button);
    opacityEffect->setOpacity(0.8);
    button->setGraphicsEffect(opacityEffect);

    return button;
}

void buildScalingUI(Ui::BasicImageView *ui, QWidget *parent)
{
    ui->scalingUI = new QWidget(parent);
    ui->scalingUI->setGeometry(347, 180, 51, 111);

    auto *layout = new QVBoxLayout(ui->scalingUI);
    layout->setContentsMargins(8, 8, 8, 8);

    ui->zoomInButton = buildZoomButton(ui->scalingUI, "zoom_in", BasicImageView::tr("Zoom In"));
    layout->addWidget(ui->zoomInButton);

    ui->zoomOutButton = buildZoomButton(ui->scalingUI, "zoom_out", BasicImageView::tr("Zoom Out"));
    QFont zoomOutFont;
    zoomOutFont.setPointSize(13);
    ui->zoomOutButton->setFont(zoomOutFont);
    layout->addWidget(ui->zoomOutButton);

    ui->scaleLabel = new QLabel("x1", ui->scalingUI);
    ui->scaleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(ui->scaleLabel);
}
} // namespace

// TODO: ScrollAreaをドラッグで操作できるようにする
BasicImageView::BasicImageView(QWidget *parent) : QWidget(parent), ui(new Ui::BasicImageView)
{
    resize(400, 300);
    setMinimumSize(64, 128);

    buildScrollArea(ui, this);
    buildScalingUI(ui, this);

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
