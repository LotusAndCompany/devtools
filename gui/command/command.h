#ifndef COMMAND_H
#define COMMAND_H

#include <QGroupBox>

namespace Ui {
class Command;
}

class Command : public QGroupBox
{
    Q_OBJECT

public:
    explicit Command(QWidget *parent = nullptr);
    ~Command();

private:
    Ui::Command *ui;
    void init();
    void reset();
};

#endif // COMMAND_H
