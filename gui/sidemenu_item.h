#ifndef SIDEMENU_ITEM_H
#define SIDEMENU_ITEM_H

#include <QPushButton>
#include "sidemenu.h"

// TODO: ツールの説明文を表示しようとすると機能が足りないのでQWidgetに変える
//       uiファイルも作る
//       uiファイル側にidの情報を設定するのは難しそう?
class SidemenuItem : public QPushButton
{
    Q_OBJECT

public:
    SidemenuItem(Sidemenu::ID id, QWidget *parent = nullptr) noexcept(false);

private:
    static const QString notConfigurableReason;

    const Sidemenu::ID id;

    void changeEvent(QEvent *event) noexcept(false) override;
};

#endif // SIDEMENU_ITEM_H
