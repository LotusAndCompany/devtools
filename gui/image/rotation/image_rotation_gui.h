#ifndef IMAGE_ROTATION_GUI_H
#define IMAGE_ROTATION_GUI_H

#include "gui/gui_tool.h"

#include <QWidget>

namespace Ui {
class ImageRotationGUI;
}
class ImageRotationInterface;

#ifdef _TEST_ImageRotationGUI
namespace Test {
class TestImageRotationGUI;
}
#endif

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
     * @param parent 親ウィジェット
     * @details UIを構築し、シグナルを接続する
     */
    explicit ImageRotationGUI(ImageRotationInterface *imageRotation, QWidget *parent = nullptr);
    ~ImageRotationGUI() override;
    ImageRotationGUI(const ImageRotationGUI &) = delete;
    ImageRotationGUI &operator=(const ImageRotationGUI &) = delete;
    ImageRotationGUI(ImageRotationGUI &&) = delete;
    ImageRotationGUI &operator=(ImageRotationGUI &&) = delete;

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
     * @brief 右に90°回転ボタンが押された時の処理
     */
    void onRotateRightButtonClicked();
    /**
     * @brief 左に90°回転ボタンが押された時の処理
     */
    void onRotateLeftButtonClicked();
    /**
     * @brief 左右反転ボタンが押された時の処理
     */
    void onFlipHorizontalButtonClicked();
    /**
     * @brief 上下反転ボタンが押された時の処理
     */
    void onFlipVerticalButtonClicked();

private:
    Ui::ImageRotationGUI *const ui;
    /// ロジック部分
    ImageRotationInterface *const imageRotation;

#ifdef _TEST_ImageRotationGUI
    friend class Test::TestImageRotationGUI;
#endif
};

#endif // IMAGE_ROTATION_GUI_H
