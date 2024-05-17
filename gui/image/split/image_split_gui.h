#ifndef IMAGE_SPLIT_GUI_H
#define IMAGE_SPLIT_GUI_H

#include <QWidget>
#include "gui/gui_tool.h"

namespace Ui {
class ImageSplitGUI;
}
class ImageSplitInterface;

#ifdef _TEST_ImageSplitGUI
#include "ui_image_split_gui.h"
namespace Test {
class TestImageSplitGUI;
}
#endif

// TODO: Undoをサポートする
/**
 * @brief 画像を分割するツールのGUI
 */
class ImageSplitGUI : public GuiTool
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param imageSplit ロジック部分
     * @param parent 親ウィジェット
     */
    explicit ImageSplitGUI(ImageSplitInterface *imageSplit, QWidget *parent = nullptr);
    ~ImageSplitGUI();

private:
    Ui::ImageSplitGUI *const ui;
    /// ロジック部分
    ImageSplitInterface *const imageSplit;

#ifdef _TEST_ImageSplitGUI
    friend class Test::TestImageSplitGUI;
#endif
};

#endif // IMAGE_SPLIT_GUI_H
