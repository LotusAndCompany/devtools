#include "contents_area.h"
#include "api_tool.h"
#include "ui_contents_area.h"

#include <QLabel>


#include "core/image/division/image_division.h"
#include "core/image/resize/image_resize.h"
#include "core/image/rotation/image_rotation.h"
#include "core/image/transparent/image_transparent.h"
#include "gui/image/division/image_division_gui.h"
#include "gui/image/resize/image_resize_gui.h"
#include "gui/image/rotation/image_rotation_gui.h"
#include "gui/image/transparent/image_transparent_gui.h"
#include "phrase_generation/phrase_generation.h"
#include "gui/command/command.h"

ContentsArea::ContentsArea(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::ContentsArea)
{
    ui->setupUi(this);

    changeContent(Sidemenu::ID::HOME);
}

ContentsArea::~ContentsArea()
{
    delete ui;
}

void ContentsArea::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void ContentsArea::onSidemenuItemChanged(Sidemenu::ID id)
{
    changeContent(id);
}

void ContentsArea::changeContent(Sidemenu::ID id)
{
    if (currentContent) {
        ui->contentsAreaLayout->removeWidget(currentContent);
        // NOTE: 解放する
        delete currentContent;
        currentContent = nullptr;
    }

    QWidget *content = nullptr;

    switch (id) {
    case Sidemenu::ID::HOME:
        content = new QLabel("(´･ω･) ﾎｰﾑﾀﾞﾖｰ", this);
        static_cast<QLabel *>(content)->setAlignment(Qt::AlignCenter);
        break;
    case Sidemenu::ID::IMAGE_RESIZE:
        content = new ImageResizeGUI(new ImageResize(), this);
        break;
    case Sidemenu::ID::IMAGE_ROTATION:
        content = new ImageRotationGUI(new ImageRotation(), this);
        break;
    case Sidemenu::ID::IMAGE_DIVISION:
        content = new ImageDivisionGUI(new ImageDivision(), this);
        break;
    case Sidemenu::ID::IMAGE_TRANSPARENT:
        content = new ImageTransparentGUI(new ImageTransparent(), this);
        break;
    case Sidemenu::ID::PHRASE_GENERATION:
        content = new phraseGeneration(this);
        break;
    case Sidemenu::ID::COMMAND_GENERATION:
        content = new Command(this);
        break;
    default:
        // NOTE: signal/slotでは例外を投げるべきではない
        content = new QLabel("Under development...", this);
        static_cast<QLabel *>(content)->setAlignment(Qt::AlignCenter);
        break;
    }

    currentContent = content;
    ui->contentsAreaLayout->addWidget(currentContent);
}
