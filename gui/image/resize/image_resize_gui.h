#ifndef IMAGE_RESIZE_GUI_H
#define IMAGE_RESIZE_GUI_H

#include <QWidget>
#include "gui/gui_tool.h"

namespace Ui {
class ImageResizeGUI;
}
class ImageResizeInterface;

#ifdef _TEST_ImageResizeGUI
#include "ui_image_resize_gui.h"
namespace Test {
class TestImageResizeGUI;
}
#endif

// TODO: Undoをサポートする
// TODO: ショートカットをサポートする
/**
 * @brief 画像をリサイズするツールのGUI
 */
class ImageResizeGUI : public GuiTool
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param ImageResize (ロジック部分)
     * @param 親ウィジェット
     * @details UIを構築し、シグナルを接続する
     */
    explicit ImageResizeGUI(ImageResizeInterface *imageResize, QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~ImageResizeGUI();

private slots:
    /**
     * @brief 読み込む画像ファイル名が選択された時の処理
     * @details 画像を読み込み、プレビューとUIを更新する
     * @param path 読み込む画像ファイル
     */
    void onLoadImageSelected(const QString &path);
    /**
     * @brief 保存先が選択された時の処理
     * @details 画像を上書き保存する
     * @param path 保存先
     */
    void onSaveImageSelected(const QString &path);
    /**
     * @brief リセットボタンが押された時に呼び出される
     * @details 拡大率の設定を初期状態に戻し、プレビューとUIを更新する
     */
    void onResetButtonClicked();
    /**
     * @brief 画像の横幅の入力欄が更新された時の処理
     * @param width 変更後の値
     * @details 入力に基づいて画像サイズを変更し、プレビューとUIを更新する
     */
    void onWidthValueChanged(int width);
    /**
     * @brief 画像の縦幅の入力欄が更新された時の処理
     * @param height 変更後の値
     * @details 入力に基づいて画像サイズを変更し、プレビューとUIを更新する
     */
    void onHeightValueChanged(int height);
    /**
     * @brief 水平方向の拡大率の入力欄が更新された時の処理
     * @param hScale 変更後の値
     * @details 入力に基づいて画像サイズを変更し、プレビューとUIを更新する
     */
    void onHorizontalScaleChanged(double hScale);
    /**
     * @brief 垂直方向の拡大率の入力欄が更新された時の処理
     * @param vScale 変更後の値
     * @details 入力に基づいて画像サイズを変更し、プレビューとUIを更新する
     */
    void onVerticalScaleChanged(double vScale);
    /**
     * @brief 縦横比固定のチェックボックスの状態が変化した時の処理
     * @param state 縦横比固定のチェックボックスの状態
     */
    void onKeepAspectRatioChanged(Qt::CheckState state);
    /**
     * @brief 画素補完のチェックボックスの状態が変化した時の処理
     * @param state 画素補完のチェックボックスの状態
     */
    void onSmoothTransformationChanged(Qt::CheckState state);

private:
    Ui::ImageResizeGUI *const ui;
    /// ロジック部分
    ImageResizeInterface *const imageResize;

    /// 縦横比固定フラグ
    bool keepAspectRatio = false;

    /**
     * @brief どの操作により画像サイズが更新されたかを表す列挙体
     */
    enum class UpdateMode {
        MIN,            ///< 最小値
        DEFAULT,        ///< loadやresetによる更新
        WIDTH_UPDATE,   ///< ui->widthValue の編集による更新
        HEIGHT_UPDATE,  ///< ui->heightValue の編集による更新
        X_SCALE_UPDATE, ///< ui->hScaleValue の編集による更新
        Y_SCALE_UPDATE, ///< ui->vScaleValue の編集による更新
        MAX,            ///<最大値
    };
    /**
     * @brief UI上の画像サイズ・拡大率を更新する
     * @param mode 更新モード
     */
    void updateUIValues(UpdateMode mode = UpdateMode::DEFAULT);
#ifdef _TEST_ImageResizeGUI
    friend class Test::TestImageResizeGUI;
#endif
};

#endif // IMAGE_RESIZE_GUI_H
