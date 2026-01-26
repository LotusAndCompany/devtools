#ifndef IMAGE_TRANSPARENT_GUI_H
#define IMAGE_TRANSPARENT_GUI_H

#include "gui/gui_tool.h"

#include <QWidget>

namespace Ui {
class ImageTransparentGUI;
}
class ImageTransparentInterface;

#ifdef _TEST_ImageTransparentGUI
namespace Test {
class TestImageTransparentGUI;
}
#endif

// TODO: Undoをサポートする
// FIXME: 処理を別のスレッドで行うようにする
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
    ~ImageTransparentGUI() override;

protected slots:
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
     * @brief 色差の計算方法(RGB/HSV/HSL)が変更された時に呼ばれる
     * @param mode 色差の計算方法(RGB/HSV/HSL)
     */
    void onColorModeTextChanged(const QString &mode);
    /**
     * @brief 画像上のピクセルが選択された時に呼ばれる
     * @param point 選択された場所
     * @param color 選択された色
     */
    void onPixelSelected(const QPoint &point, const QColor &color);
    /**
     * @brief 許容範囲の値が変更された時に呼ばれる
     * @param tolerance 許容範囲の値
     */
    void onToleranceValueChanged(double tolerance);
    /**
     * @brief 透明度の値が変更された時に呼ばれる
     * @param transparency 透明度の値
     */
    void onTransparencyValueChanged(double transparency);
    /**
     * @brief 連続領域選択のチェックボックスが変化した時に呼ばれる
     * @param state 連続領域選択のチェックボックスの状態
     */
    void onContiguousAreaCheckStateChanged(Qt::CheckState state);

private:
    Ui::ImageTransparentGUI *const ui;
    /// ロジック部分
    ImageTransparentInterface *const imageTransparent;

    /// 連続領域を透明化するフラグ
    bool onlyContiguousArea = true;

#ifdef _TEST_ImageTransparentGUI
    friend class Test::TestImageTransparentGUI;
#endif
};

#endif // IMAGE_TRANSPARENT_GUI_H
