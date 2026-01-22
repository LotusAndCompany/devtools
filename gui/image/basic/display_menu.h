#ifndef BASIC_DISPLAY_MENU_H
#define BASIC_DISPLAY_MENU_H

#include <QPixmap>
#include <QWidget>

namespace Ui {
class BasicDisplayMenu;
}

/**
 * @brief 画像編集系のGUIツールで画像を表示する部分
 */
class BasicDisplayMenu : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     * @details UIを構築し、シグナルを接続する
     */
    explicit BasicImageView(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    virtual ~BasicImageView();

    /**
     * @brief 画像データを設定する。空の画像データを指定すると "No Image" になる
     * @param pixmap 画像データ
     * @param reset `true`なら拡大率をリセットする
     */
    void setPixmap(const QPixmap &pixmap, bool reset = false);

protected:
    Ui::BasicImageView *const ui;

    /// 実際の拡大率
    double scale = 1.0;

    /// 拡大縮小前の画像データ。 setPixmap() によってのみ変更される
    QPixmap original;

    /**
     * @brief scale を変更し、UIに反映する
     * @param newScale 新しい scale の値
     */
    virtual void updateScale(double newScale); // NOTE: 関数名を改めた方が良いかも

private slots:
    /**
     * @brief 拡大ボタンが押された時の処理
     */
    void onZoomInButtonPressed();
    /**
     * @brief 縮小ボタンが押された時の処理
     */
    void onZoomOutButtonPressed();

private:

};

#endif // BASIC_DISPLAY_MENU_H

