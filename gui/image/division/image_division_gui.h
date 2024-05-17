#ifndef IMAGE_DIVISION_GUI_H
#define IMAGE_DIVISION_GUI_H

#include <QWidget>
#include "gui/gui_tool.h"

namespace Ui {
class ImageDivisionGUI;
}
class ImageDivisionInterface;

#ifdef _TEST_ImageDivisionGUI
#include "ui_image_division_gui.h"
namespace Test {
class TestImageDivisionGUI;
}
#endif

// TODO: Undoをサポートする
/**
 * @brief 画像を分割するツールのGUI
 */
class ImageDivisionGUI : public GuiTool
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param imageDivision ロジック部分
     * @param parent 親ウィジェット
     */
    explicit ImageDivisionGUI(ImageDivisionInterface *imageDivision, QWidget *parent = nullptr);
    ~ImageDivisionGUI();

private:
    Ui::ImageDivisionGUI *const ui;
    /// ロジック部分
    ImageDivisionInterface *const imageDivision;

#ifdef _TEST_ImageDivisionGUI
    friend class Test::TestImageDivisionGUI;
#endif
};

#endif // IMAGE_DIVISION_GUI_H
