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
    void selectedGitFunction();
    void selectedDockerFunction();
    void generate();
    void gitCommandGenerate();
    void dockerCommandGenerate();
    void reset();
    void clear();
    void copy();
    void adjustCommandBoxWidth();    
};

#endif // COMMAND_H
