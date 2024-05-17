#ifndef BASIC_IMAGE_VIEW_CONTROL_H
#define BASIC_IMAGE_VIEW_CONTROL_H

#include <QWidget>

#ifdef _TEST_BasicImageViewControl
#include "ui_control.h"

namespace Test {
class TestBasicImageViewControl;
}
#else
namespace Ui {
class BasicImageViewControl;
}
#endif

/**
 * @brief 画像編集系GUIツールの共通する要素。load/reset/saveボタン
 */
class BasicImageViewControl : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     * @details UIを構築し、シグナルを接続する
     */
    explicit BasicImageViewControl(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~BasicImageViewControl();

    /**
     * @brief save/loadで開くダイアログの種類
     */
    enum class DialogType {
        MIN,           ///< 最小値
        SELECT_FILE,   ///< ファイルを選択するダイアログ
        SELECT_FOLDER, ///< フォルダを選択するダイアログ
        MAX,           ///< 最大値
    } saveFileDailogType
        = DialogType::SELECT_FILE;

signals:
    /**
     * @brief reset ボタンが押された時に発せられる
     */
    void resetButtonClicked();

    /**
     * @brief 読み込むファイルが選択された時に発せられる
     * @param path ファイル名
     */
    void loadFileSelected(const QString &path);
    /**
     * @brief 保存先が選択された時に発せられる
     * @param path 保存先
     */
    void saveFileSelected(const QString &path);

private slots:
    /**
     * @brief 保存ボタンが押された時の処理
     */
    void onSaveButtonClicked();
    /**
     * @brief 読み込みボタンが押された時の処理
     */
    void onLoadButtonClicked();

private:
    static const QString invalidDialogType;

    Ui::BasicImageViewControl *const ui;

#ifdef _TEST_BasicImageViewControl
    friend class Test::TestBasicImageViewControl;
#endif
};

#endif // BASIC_IMAGE_VIEW_CONTROL_H
