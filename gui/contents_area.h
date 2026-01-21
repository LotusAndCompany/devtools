#ifndef CONTENTS_AREA_H
#define CONTENTS_AREA_H

#include "sidemenu.h"

#include <QFrame>

namespace Ui {
class ContentsArea;
}

class dbMain;

/**
 * @brief メインウィンドウ右側の領域
 */
class ContentsArea : public QFrame
{
    Q_OBJECT

    /// UI(レイアウトの指定のみ)
    Ui::ContentsArea *const ui;

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit ContentsArea(QWidget *parent = nullptr);
    /**
     * デストラクタ
     */
    ~ContentsArea();

public slots:
    /**
     * @brief サイドメニューのボタンを押した時に呼び出される
     * @param id サイドメニューID
     * @details Sidemenu::itemSelected(Sidemenu::ID id) と接続される
     *
     * @sa MainWindow::MainWindow(QWidget *parent)
     */
    void onSidemenuItemChanged(Sidemenu::ID id);

private:
    /**
     * @brief 現在表示中のウィジェット
     */
    QWidget *currentContent = nullptr;

    /**
     * @brief DBツールのキャッシュ（状態を保持するため）
     */
    dbMain *cachedDbMain = nullptr;

    /**
     * @brief イベント処理
     * @details 言語の変更があった場合にUIに反映する
     * @param event 発生したイベント
     */
    void changeEvent(QEvent *event) override;
    /**
     * @brief currentContent を指定された Sidemenu::ID の物に差し替える
     * @param id
     */
    void changeContent(Sidemenu::ID id);
};
#endif // CONTENTS_AREA_H
