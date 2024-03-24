#ifndef CONTENTS_AREA_H
#define CONTENTS_AREA_H

#include <QWidget>

namespace Ui {
class ContentsArea;
}

class ContentsArea : public QWidget
{
    Q_OBJECT

    Ui::ContentsArea* const ui;

public:
    explicit ContentsArea(QWidget *parent = nullptr);
    virtual ~ContentsArea();

private:
    void changeEvent(QEvent *event) override;
};

#endif // CONTENTS_AREA_H
