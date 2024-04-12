#ifndef BASIC_IMAGE_VIEW_CONTROL_H
#define BASIC_IMAGE_VIEW_CONTROL_H

#include <QWidget>

namespace Ui {
class BasicImageViewControl;
}

/**
 * @brief 画像編集系GUIツールの虚移す要素。load/reset/saveボタン
 */
class BasicImageViewControl : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit BasicImageViewControl(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~BasicImageViewControl();

signals:
    /**
     * @brief load ボタンが押された時に発せられる
     */
    //void loadButtonClicked();
    /**
     * @brief reset ボタンが押された時に発せられる
     */
    void resetButtonClicked();
    /**
     * @brief save ボタンが押された時に発せられる
     */
    //void saveButtonClicked();

    void loadFileSelected(const QString &path);
    void saveFileSelected(const QString &path);

private slots:
    void onSaveButtonClicked();
    void onLoadButtonClicked();

private:
    Ui::BasicImageViewControl *const ui;
    //bool saveDialogOpened = false, loadDialogOpened = false;
};

#endif // BASIC_IMAGE_VIEW_CONTROL_H
