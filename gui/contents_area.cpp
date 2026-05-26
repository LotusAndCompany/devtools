#include "contents_area.h"

#include "api_tool.h"
#include "core/data_conversion/data_conversion.h"
#include "core/image/division/image_division.h"
#include "core/image/resize/image_resize.h"
#include "core/image/rotation/image_rotation.h"
#include "core/image/transparent/image_transparent.h"
#include "gui/command/command.h"
#include "gui/data_conversion/data_conversion_gui.h"
#include "gui/db_tool/main/db_main.h"
#include "gui/image/division/image_division_gui.h"
#include "gui/image/resize/image_resize_gui.h"
#include "gui/image/rotation/image_rotation_gui.h"
#include "gui/image/transparent/image_transparent_gui.h"
#include "gui/qr_code/qr_code_generation_gui.h"
#include "gui/welcome_page.h"
#include "phrase_generation/phrase_generation.h"

#include <QLabel>
#include <QVBoxLayout>

ContentsArea::ContentsArea(QWidget *parent)
    : QFrame(parent), m_contentsAreaLayout(new QVBoxLayout(this))
{
    setFrameShape(QFrame::StyledPanel);

    m_contentsAreaLayout->setSpacing(0);
    m_contentsAreaLayout->setContentsMargins(6, 6, 6, 6);

    changeContent(Sidemenu::ID::WELCOME);
}

void ContentsArea::onSidemenuItemChanged(Sidemenu::ID id)
{
    changeContent(id);
}

bool ContentsArea::openSQLiteFileInDbTool(const QString &filePath)
{
    changeContent(Sidemenu::ID::DB_TOOL);
    if (cachedDbMain == nullptr) {
        return false;
    }

    return cachedDbMain->connectSQLiteFile(filePath);
}

void ContentsArea::changeContent(Sidemenu::ID id)
{
    if (currentContent != nullptr) {
        m_contentsAreaLayout->removeWidget(currentContent);
        // DBツールはキャッシュするので削除しない
        if (currentContent != cachedDbMain) {
            delete currentContent;
        } else {
            currentContent->hide();
        }
        currentContent = nullptr;
    }

    QWidget *content = nullptr;

    switch (id) {
    case Sidemenu::ID::WELCOME:
        content = new WelcomePage(this);
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
    case Sidemenu::ID::HTTP_REQUEST:
        content = new api_tool(this);
        break;
    case Sidemenu::ID::DATA_CONVERSION:
        content = new DataConversionGUI(new DataConversion(), this);
        break;
    case Sidemenu::ID::DB_TOOL:
        // DBツールはキャッシュして状態を保持
        if (cachedDbMain == nullptr) {
            cachedDbMain = new dbMain(this);
        }
        content = cachedDbMain;
        content->show();
        break;
    case Sidemenu::ID::QR_CODE_GENERATION:
        content = new QRCodeGenerationGUI(this);
        break;
    default:
        // NOTE: signal/slotでは例外を投げるべきではない
        content = new QLabel("Under development...", this);
        dynamic_cast<QLabel *>(content)->setAlignment(Qt::AlignCenter);
        break;
    }

    currentContent = content;
    m_contentsAreaLayout->addWidget(currentContent);
}
