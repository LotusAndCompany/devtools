#ifndef IMAGE_ROTATION_GUI_H
#define IMAGE_ROTATION_GUI_H

#include <QWidget>
#include "gui/gui_tool.h"

namespace Ui {
class ImageRotationGUI;
}
class ImageRotationInterface;

// TODO: Undoをサポートする
/**
 * @brief 画像を回転するツールのGUI
 */
class ImageRotationGUI : public GuiTool
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param imageRotation ロジック部分
     * @param 親ウィジェット
     * @details UIを構築し、シグナルを接続する
     */
    explicit ImageRotationGUI(ImageRotationInterface *imageRotation, QWidget *parent = nullptr);
    ~ImageRotationGUI();

private:
    Ui::ImageRotationGUI *const ui;
    /// ロジック部分
    ImageRotationInterface *const imageRotation;
};

#endif // IMAGE_ROTATION_GUI_H
