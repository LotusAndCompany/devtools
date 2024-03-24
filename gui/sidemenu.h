#ifndef SIDEMENU_H
#define SIDEMENU_H

#include <QWidget>

class QButtonGroup;
class QAbstractButton;
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
    ~Sidemenu();

    enum class ItemID {
        UNDEFINED,
        HOME,
        SAMPLE_0,
        SAMPLE_1,
        SAMPLE_2,
        SAMPLE_3,
        MAX, // これ以降値を追加しないこと
    };
    constexpr static const int ItemID_UNDEFINED = static_cast<int>(ItemID::UNDEFINED);
    constexpr static const int ItemID_MAX = static_cast<int>(ItemID::MAX);

signals:
    void itemSelected(ItemID id);

private:
    void changeEvent(QEvent *event) override;
    void registerItem(ItemID id);

private slots:
    void onButtonToggled(int id, bool checked);
};

#endif // SIDEMENU_H
