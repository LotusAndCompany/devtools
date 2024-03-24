#ifndef CONTENTS_AREA_H
#define CONTENTS_AREA_H

#include <QWidget>
#include "gui/sidemenu.h"

namespace Ui {
class ContentsArea;
}

class ContentsArea : public QWidget
{
    Q_OBJECT

    Ui::ContentsArea* const ui;
    // TODO: コンテンツページ用の基底クラス作る
    QWidget *currentContent = nullptr;

public:
    explicit ContentsArea(QWidget *parent = nullptr);
    virtual ~ContentsArea();

public slots:
    void onSidemenuItemChanged(Sidemenu::ItemID id);

private:
    void changeEvent(QEvent *event) override;
    void changeContent(Sidemenu::ItemID id);
};

#endif // CONTENTS_AREA_H
