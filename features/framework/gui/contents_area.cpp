#include "contents_area.h"

#include "features/command/gui/command.h"
#include "features/data_conversion/core/data_conversion.h"
#include "features/data_conversion/gui/data_conversion_gui.h"
#include "features/db_tool/gui/db_main/db_main.h"
#include "features/framework/gui/welcome_page.h"
#include "features/http_request/gui/api_tool.h"
#include "features/image/gui/basic/image_tools_unified_gui.h"
#include "features/markdown_preview/core/markdown_preview.h"
#include "features/markdown_preview/gui/markdown_preview_gui.h"
#include "features/phrase_generation/gui/phrase_generation.h"
#include "features/qr_code/gui/qr_code_generation_gui.h"
#include "features/regex_tool/gui/regex_tester_gui.h"

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
    case Sidemenu::ID::IMAGE_ALL_IN_ONE:
        content = new ImageToolsUnifiedGUI(this);
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
    case Sidemenu::ID::MARKDOWN_PREVIEW:
        content = new MarkdownPreviewGUI(new MarkdownPreview(), this);
        break;
    case Sidemenu::ID::REGEX_TESTER:
        content = new devtools::RegexTesterGUI(this);
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
