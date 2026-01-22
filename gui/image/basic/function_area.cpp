#include "function_area.h"
#include "ui_function_area.h"
//#include "core/image/resize/image_resize.h"
//#include "core/image/division/image_division.h"
//#include "core/image/rotation/image_rotation.h"


BasicImageFunctionArea::BasicImageFunctionArea(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BasicImageFunctionArea)
{
    //ui->setupUi(this);

    QFrame *frame = new QFrame(this);
    frame->setObjectName("basicImageFunctionFrame");
    frame->setFrameShape(QFrame::Box);
    frame->setFrameShadow(QFrame::Plain);
    frame->setLineWidth(1);
    frame->setStyleSheet(
        "#basicImageFunctionFrame {"
        "  border: 1px solid #000;"
        "}"
        );

    auto outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);
    outerLayout->addWidget(frame);

    ui->setupUi(frame);

    // ImageResize インスタンスを作成
    //ImageResizeInterface* imageResizeInterface = new ImageResize(this);

    // ImageRotation インスタンスを作成
    //ImageRotationInterface* imageRotationInterface = new ImageRotation(this);

    // ImageDivision インスタンスを作成
    //ImageDivisionInterface* imageDivisionInterface = new ImageDivision(this);

    //ui->resize = new ImageResizeGUI(imageResizeInterface, this);
    //ui->resize = new ImageResizeGUI(this);
    //ui->rotation = new ImageRotationGUI(imageRotationInterface, this);
    //ui->division = new ImageDivisionGUI(imageDivisionInterface, this);

    connect(ui->functionGroup1, &QGroupBox::toggled,
            ui->resize, &QWidget::setVisible);
}

BasicImageFunctionArea::~BasicImageFunctionArea()
{
    delete ui;
}
