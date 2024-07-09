#ifndef IMAGE_TRANSPARENT_GUI_H
#define IMAGE_TRANSPARENT_GUI_H

#include <QWidget>
#include "gui/gui_tool.h"

namespace Ui {
class ImageTransparentGUI;
}
class ImageTransparentInterface;

#ifdef _TEST_ImageTransparentGUI
#include "ui_image_transparent_gui.h"
namespace Test {
class TestImageTransparentGUI;
}
#endif

// TODO: Undoをサポートする
/**
 * @brief 画像を透明化するツールのGUI
 */
class ImageTransparentGUI : public GuiTool
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param imageTransparent ロジック部分
     * @param parent 親ウィジェット
     * @details UIを構築し、シグナルを接続する
     */
    explicit ImageTransparentGUI(ImageTransparentInterface *imageTransparent,
                                 QWidget *parent = nullptr);
    ~ImageTransparentGUI();

protected slots:
    void onColorModeTextChanged(const QString &);
    void onColorChanged(const QColor &);
    void onToleranceValueChanged(double);
    void onTransparencyValueChanged(double);
    void onAutoColorPickCheckStateChanged(Qt::CheckState);
    void onContiguousAreaCheckStateChanged(Qt::CheckState);

private:
    Ui::ImageTransparentGUI *const ui;
    /// ロジック部分
    ImageTransparentInterface *const imageTransparent;
    /// 連続領域を透明化するフラグ
    bool onlyContiguousArea = true;
};

#endif // IMAGE_TRANSPARENT_GUI_H
