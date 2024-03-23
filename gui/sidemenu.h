#ifndef SIDEMENU_H
#define SIDEMENU_H

#include <QWidget>

namespace Ui {
class Sidemenu;
}

class Sidemenu : public QWidget
{
    Q_OBJECT

    Ui::Sidemenu *const ui;

public:
    explicit Sidemenu(QWidget *parent = nullptr);
    virtual ~Sidemenu();
};

#endif // SIDEMENU_H
