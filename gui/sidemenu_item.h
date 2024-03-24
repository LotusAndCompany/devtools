#ifndef SIDEMENU_ITEM_H
#define SIDEMENU_ITEM_H

#include <QPushButton>

class SidemenuItem : public QPushButton
{
    Q_OBJECT

public:
    explicit SidemenuItem(QWidget *parent = nullptr);
    SidemenuItem(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
    virtual ~SidemenuItem() = default;

private:
    void mouseReleaseEvent(QMouseEvent *e) override;
    void changeEvent(QEvent *event) override;
};

#endif // SIDEMENU_ITEM_H
