#include "image_rotation_gui.h"

#include "core/image/rotation/image_rotation.h"
#include "gui/image/basic/control.h"
#include "gui/image/basic/image_view.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

namespace {
QPushButton *buildActionButton(QWidget *parent, const QString &text, const QString &toolTip,
                               const QString &iconName)
{
    auto *button = new QPushButton(QIcon::fromTheme(iconName), text, parent);
    button->setToolTip(toolTip);
    button->setStyleSheet("text-align: left;");
    button->setIconSize(QSize(16, 16));
    return button;
}

void buildButtonColumn(Ui::ImageRotationGUI *ui, QWidget *parent, QVBoxLayout *layout)
{
    auto *column = new QVBoxLayout();
    column->setContentsMargins(16, 0, 16, 0);

    ui->rotateRightButton =
        buildActionButton(parent, ImageRotationGUI::tr("Rotate Clockwise"),
                          ImageRotationGUI::tr("Rotate clockwise by 90°"), "clockwise");
    ui->rotateRightButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    column->addWidget(ui->rotateRightButton);

    ui->rotateLeftButton =
        buildActionButton(parent, ImageRotationGUI::tr("Rotate Anti-clockwise"),
                          ImageRotationGUI::tr("Rotate anti-clockwise by 90°"), "anticlockwise");
    column->addWidget(ui->rotateLeftButton);

    ui->hFlipButton =
        buildActionButton(parent, ImageRotationGUI::tr("Flip Horizontal"),
                          ImageRotationGUI::tr("Flip horizontally"), "flip_horizontal");
    column->addWidget(ui->hFlipButton);

    ui->vFlipButton = buildActionButton(parent, ImageRotationGUI::tr("Flip Vertical"),
                                        ImageRotationGUI::tr("Flip vertically"), "flip_vertical");
    column->addWidget(ui->vFlipButton);

    layout->addLayout(column);
}

void buildUiArea(Ui::ImageRotationGUI *ui, QWidget *parent, QHBoxLayout *rootLayout)
{
    auto *uiArea = new QFrame(parent);
    uiArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    uiArea->setMinimumWidth(240);
    uiArea->setFrameShape(QFrame::StyledPanel);
    uiArea->setFrameShadow(QFrame::Raised);

    auto *layout = new QVBoxLayout(uiArea);

    buildButtonColumn(ui, uiArea, layout);

    layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    ui->basicImageViewControl = new BasicImageViewControl(uiArea);
    layout->addWidget(ui->basicImageViewControl);

    rootLayout->addWidget(uiArea);
}
} // namespace

ImageRotationGUI::ImageRotationGUI(ImageRotationInterface *imageRotation, QWidget *parent)
    : GuiTool(parent), imageRotation(imageRotation), ui(new Ui::ImageRotationGUI)
{
    resize(400, 300);

    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    ui->imageView = new BasicImageView(this);
    ui->imageView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    rootLayout->addWidget(ui->imageView);

    buildUiArea(ui, this, rootLayout);

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (imageRotation->parent() == nullptr) {
        imageRotation->setParent(this);
    }

    connect(ui->imageView, &BasicImageView::loadFileSelected, ui->basicImageViewControl,
            &BasicImageViewControl::loadFileSelected);
    connect(ui->basicImageViewControl, &BasicImageViewControl::loadFileSelected, this,
            &ImageRotationGUI::onLoadImageSelected);
    connect(ui->basicImageViewControl, &BasicImageViewControl::saveFileSelected, this,
            &ImageRotationGUI::onSaveImageSelected);
    connect(ui->basicImageViewControl, &BasicImageViewControl::resetButtonClicked, this,
            &ImageRotationGUI::onResetButtonClicked);

    connect(ui->rotateLeftButton, &QPushButton::clicked, this,
            &ImageRotationGUI::onRotateLeftButtonClicked);
    connect(ui->rotateRightButton, &QPushButton::clicked, this,
            &ImageRotationGUI::onRotateRightButtonClicked);
    connect(ui->hFlipButton, &QPushButton::clicked, this,
            &ImageRotationGUI::onFlipHorizontalButtonClicked);
    connect(ui->vFlipButton, &QPushButton::clicked, this,
            &ImageRotationGUI::onFlipVerticalButtonClicked);
}

ImageRotationGUI::~ImageRotationGUI()
{
    delete ui;
}

void ImageRotationGUI::onLoadImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    imageRotation->load(path);
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()), true);

    // TODO: load()の結果に応じて何かメッセージを出す
}

void ImageRotationGUI::onSaveImageSelected(const QString &path)
{
    qDebug() << "path:" << path;

    if (!imageRotation->overwriteSave(path)) {
        QMessageBox::critical(this, tr("Save Failed"), tr("Failed to save the image."));
    }
}

void ImageRotationGUI::onResetButtonClicked()
{
    imageRotation->reset();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}

void ImageRotationGUI::onRotateRightButtonClicked()
{
    if (imageRotation->original().isNull()) {
        return;
    }

    imageRotation->rotateDegrees(-90);
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}

void ImageRotationGUI::onRotateLeftButtonClicked()
{
    if (imageRotation->original().isNull()) {
        return;
    }

    imageRotation->rotateDegrees(90);
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}

void ImageRotationGUI::onFlipHorizontalButtonClicked()
{
    if (imageRotation->original().isNull()) {
        return;
    }

    imageRotation->flipHorizontal();
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}

void ImageRotationGUI::onFlipVerticalButtonClicked()
{
    if (imageRotation->original().isNull()) {
        return;
    }

    imageRotation->flipVertical();
    imageRotation->update();

    ui->imageView->setPixmap(QPixmap::fromImage(imageRotation->current()));
}
