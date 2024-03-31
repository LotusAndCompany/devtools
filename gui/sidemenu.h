#ifndef SIDEMENU_H
#define SIDEMENU_H

#include <QWidget>
#include "core/tool/tool_id_fields.h"

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

    enum class ID {
        UNDEFINED,
        HOME,
        TOOL_ID_FIELDS(), // ここに展開する
        MAX,
    };
    constexpr static const int ID_UNDEFINED = static_cast<int>(ID::UNDEFINED);
    constexpr static const int ID_MAX = static_cast<int>(ID::MAX);

signals:
    void itemSelected(ID id);

private:
    void changeEvent(QEvent *event) override;
    void registerItem(ID id);

private slots:
    void onButtonToggled(int id, bool checked);
};

#endif // SIDEMENU_H
