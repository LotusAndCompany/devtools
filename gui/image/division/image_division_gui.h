#ifndef IMAGE_DIVISION_GUI_H
#define IMAGE_DIVISION_GUI_H

#include "gui/gui_tool.h"

#include <QWidget>

class QAbstractButton;

namespace Ui {
class ImageDivisionGUI;
}
class ImageDivisionInterface;

#ifdef _TEST_ImageDivisionGUI
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
    ~ImageDivisionGUI() override;

private slots:
    /**
     * @brief 読み込む画像ファイル名が選択された時の処理
     * @details 画像を読み込み、プレビューとUIを更新する
     * @param path 読み込む画像ファイル
     */
    void onLoadImageSelected(const QString &path);
    /**
     * @brief 保存先フォルダが選択された時の処理
     * @details 同名の画像が存在する場合は確認ダイアログを出す
     * @param path 保存先
     */
    void onSaveLocationSelected(const QString &path);
    /**
     * @brief リセットボタンが押された時に呼び出される
     * @details 分割の設定を初期状態に戻し、プレビューとUIを更新する
     */
    void onResetButtonClicked();
    /**
     * @brief 分割数指定/サイズ指定切り替え時の処理
     * @param button 押されたボタン
     */
    void onDivisionModeClicked(QAbstractButton *button);
    /**
     * @brief 横の分割数の値が変更された時の処理
     * @param hDiv 横の分割数
     */
    void onHorizontalDivisionValueChanged(int hDiv);
    /**
     * @brief 縦の分割数の値が変更された時の処理
     * @param vDiv 横の分割数
     */
    void onVerticalDivisionValueChanged(int vDiv);
    /**
     * @brief 横幅の値が変更された時の処理
     * @param width 横幅
     */
    void onWidthValueChanged(int width);
    /**
     * @brief 縦幅の値が変更された時の処理
     * @param height 縦幅
     */
    void onHeightValueChanged(int height);
    /**
     * @brief 余りを切り捨てる設定が変更された時の処理
     * @param checked `true`なら切り捨てる
     */
    void onDiscardRemainderToggled(bool checked);

private:
    Ui::ImageDivisionGUI *const ui;
    /// ロジック部分
    ImageDivisionInterface *const imageDivision;

    /**
     * @brief UI上の数値を更新する処理
     */
    void updateUI();

#ifdef _TEST_ImageDivisionGUI
    friend class Test::TestImageDivisionGUI;
#endif
};

#endif // IMAGE_DIVISION_GUI_H
