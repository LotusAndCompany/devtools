#ifndef SIDEMENU_H
#define SIDEMENU_H

#include <QWidget>

class QButtonGroup;

namespace Ui {
class Sidemenu;
}

class Sidemenu : public QWidget
{
    Q_OBJECT

    Ui::Sidemenu *const ui;

    QButtonGroup *const buttonGroup;

public:
    explicit Sidemenu(QWidget *parent = nullptr);
    virtual ~Sidemenu();

    enum class ItemID {
        HOME,
        SAMPLE_0,
        SAMPLE_1,
        SAMPLE_2,
    };

private:
    void changeEvent(QEvent *event) override;
};

#endif // SIDEMENU_H
