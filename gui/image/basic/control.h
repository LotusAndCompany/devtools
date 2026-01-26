#ifndef BASIC_IMAGE_VIEW_CONTROL_H
#define BASIC_IMAGE_VIEW_CONTROL_H

#include <QWidget>

namespace Ui {
class BasicImageViewControl;
}

#ifdef _TEST_BasicImageViewControl
namespace Test {
class TestBasicImageViewControl;
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
    ~BasicImageViewControl() override;
    BasicImageViewControl(const BasicImageViewControl &) = delete;
    BasicImageViewControl &operator=(const BasicImageViewControl &) = delete;
    BasicImageViewControl(BasicImageViewControl &&) = delete;
    BasicImageViewControl &operator=(BasicImageViewControl &&) = delete;

    /**
     * @brief save/loadで開くダイアログの種類
     */
    enum class DialogType : uint8_t {
        MIN,                                        ///< 最小値
        SELECT_FILE,                                ///< ファイルを選択するダイアログ
        SELECT_FOLDER,                              ///< フォルダを選択するダイアログ
        MAX,                                        ///< 最大値
    } saveFileDialogType = DialogType::SELECT_FILE; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

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
    void onLoadButtonClicked() const;
    /**
     * @brief 読み込むファイルが選択された時の処理
     * @param path ファイル名
     */
    void onLoadFileSelected(const QString &path);

private:
    /// DialogTypeが不正
    static const QString invalidDialogType;

    Ui::BasicImageViewControl *const ui;

    /// 読み込んだファイル名
    QString fileName;

#ifdef _TEST_BasicImageViewControl
    friend class Test::TestBasicImageViewControl;
#endif
};

#endif // BASIC_IMAGE_VIEW_CONTROL_H
