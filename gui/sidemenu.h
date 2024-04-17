#ifndef SIDEMENU_H
#define SIDEMENU_H

#include <QMap>
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

public:
    explicit Sidemenu(QWidget *parent = nullptr) noexcept(false);
    ~Sidemenu();

    enum class ID {
        MIN,
        TOOL_ID_FIELDS(), // ここに展開する
        HOME,             // ↑がTool::IDと一致した方が都合が良い
        MAX,
    };
    constexpr static const int ID_MIN = static_cast<int>(ID::MIN);
    constexpr static const int ID_MAX = static_cast<int>(ID::MAX);

    static void validateID(ID id) noexcept(false);
    static const QIcon icon(ID id) noexcept(false);

signals:
    void itemSelected(ID id);

private:
    static const QString invalidSidemenuIDReason;

    Ui::Sidemenu *const ui;

    QButtonGroup *const buttonGroup;
    QList<SidemenuItem *> sidemenuItems;

    void changeEvent(QEvent *event) override;
    void registerItem(ID id) noexcept(false);

private slots:
    void onButtonToggled(int id, bool checked);
};

#endif // SIDEMENU_H
