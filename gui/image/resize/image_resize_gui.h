#ifndef IMAGE_RESIZE_GUI_H
#define IMAGE_RESIZE_GUI_H

#include <QWidget>
#include "gui/gui_tool.h"

namespace Ui {
class ImageResizeGUI;
}
class ImageResizeInterface;

// TODO: Undoをサポートする
// TODO: ショートカットをサポートする
/**
 * @brief イメージをリサイズするツールのGUI
 */
class ImageResizeGUI : public GuiTool
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param ImageResize (ロジック部分)
     * @param 親ウィジェット
     */
    explicit ImageResizeGUI(ImageResizeInterface *imageResize, QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~ImageResizeGUI();

private slots:
    void onLoadImageSelected(const QString &path);
    void onSaveImageSelected(const QString &path);
    void onResetButtonClicked();
    void onWidthValueEditingFinished();
    void onHeightValueEditingFinished();
    void onHorizontalScaleEditingFinished();
    void onVerticalScaleEditingFinished();
    void onKeepAspectRatioChanged(Qt::CheckState state);
    void onSmoothTransformationChanged(Qt::CheckState state);

private:
    Ui::ImageResizeGUI *const ui;
    ImageResizeInterface *const imageResize;

    bool keepAspectRatio;

    enum class UpdateMode {
        /// 最小値
        MIN,
        /// loadやresetによる更新
        DEFAULT,
        /// ui->widthValue の編集による更新
        WIDTH_UPDATE,
        /// ui->heightValue の編集による更新
        HEIGHT_UPDATE,
        /// ui->hScaleValue の編集による更新
        X_SCALE_UPDATE,
        /// ui->vScaleValue の編集による更新
        Y_SCALE_UPDATE,
        /// 最大値
        MAX,
    };
    void updateUIValues(UpdateMode mode = UpdateMode::DEFAULT);
};

#endif // IMAGE_RESIZE_GUI_H
