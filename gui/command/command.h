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
    void selectedCategory();
    void selectedFunction();
    void selectedOption();
    void generate();
    void reset();
    void clear();
    void copy();
    void adjustCommandBoxWidth();
};

#endif // COMMAND_H
