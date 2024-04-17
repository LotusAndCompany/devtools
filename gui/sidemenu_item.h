#ifndef SIDEMENU_ITEM_H
#define SIDEMENU_ITEM_H

#include <QPushButton>
#include "sidemenu.h"

// TODO: ツールの説明文を表示しようとすると機能が足りないのでQWidgetに変える
//       uiファイルも作る
//       uiファイル側にidの情報を設定するのは難しそう?
/**
 * @brief サイドメニューのボタン
 */
class SidemenuItem : public QPushButton
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param id ID
     * @param parent 親ウィジェット
     */
    SidemenuItem(Sidemenu::ID id, QWidget *parent = nullptr) noexcept(false);

private:
    /// 例外に設定するメッセージ
    static const QString notConfigurableReason;

    const Sidemenu::ID id;

    /**
     * @brief 変更系のイベント処理
     * @param event 処理するイベント
     * @details 言語の変更を反映する
     */
    void changeEvent(QEvent *event) noexcept(false) override;
};

#endif // SIDEMENU_ITEM_H
