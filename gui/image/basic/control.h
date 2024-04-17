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
     */
    explicit BasicImageViewControl(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~BasicImageViewControl();

signals:
    /**
     * @brief reset ボタンが押された時に発せられる
     */
    void resetButtonClicked();

    void loadFileSelected(const QString &path);
    void saveFileSelected(const QString &path);

private slots:
    void onSaveButtonClicked();
    void onLoadButtonClicked();

private:
    Ui::BasicImageViewControl *const ui;

#ifdef _TEST_BasicImageViewControl
    friend class Test::TestBasicImageViewControl;
#endif
};

#endif // BASIC_IMAGE_VIEW_CONTROL_H
