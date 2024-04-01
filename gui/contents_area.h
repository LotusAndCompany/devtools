#ifndef CONTENTS_AREA_H
#define CONTENTS_AREA_H

#include <QFrame>
#include "sidemenu.h"

namespace Ui {
class ContentsArea;
}

class ContentsArea : public QFrame
{
    Q_OBJECT

    Ui::ContentsArea *const ui;

    // TODO: GuiTool* に置き換える
    QWidget *currentContent = nullptr;

public:
    explicit ContentsArea(QWidget *parent = nullptr);
    ~ContentsArea();

public slots:
    void onSidemenuItemChanged(Sidemenu::ID id);

private:
    void changeEvent(QEvent *event) override;
    void changeContent(Sidemenu::ID id);
};

#endif // CONTENTS_AREA_H
