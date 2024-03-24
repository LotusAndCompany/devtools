#ifndef SIDEMENU_H
#define SIDEMENU_H

#include <QWidget>

class QButtonGroup;
class SidemenuItem;

namespace Ui {
class Sidemenu;
}

class Sidemenu : public QWidget
{
    Q_OBJECT

    Ui::Sidemenu *const ui;

    QButtonGroup *const buttonGroup;
    QList<SidemenuItem *> sidemenuItems;

public:
    explicit Sidemenu(QWidget *parent = nullptr);
    virtual ~Sidemenu();

    enum class ItemID {
        UNDEFINED,
        HOME,
        SAMPLE_0,
        SAMPLE_1,
        SAMPLE_2,
        SAMPLE_3,
    };

private:
    void changeEvent(QEvent *event) override;
    void registerItem(ItemID id);
};

#endif // SIDEMENU_H
