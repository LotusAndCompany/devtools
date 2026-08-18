#ifndef COMMAND_H
#define COMMAND_H

#include <QGroupBox>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTextBrowser;

class Command : public QGroupBox
{
    Q_OBJECT

public:
    explicit Command(QWidget *parent = nullptr);
    ~Command() override = default;
    Command(const Command &) = delete;
    Command &operator=(const Command &) = delete;
    Command(Command &&) = delete;
    Command &operator=(Command &&) = delete;

private:
    void buildUi();
    void init();
    void selectedCategory();
    void selectedFunction();
    void selectedOption();
    void generate();
    void reset();
    void clear();
    void copy();
    void adjustCommandBoxWidth();

    QComboBox *category_list{nullptr};
    QLabel *functions_label{nullptr};
    QComboBox *functions_list{nullptr};
    QLabel *option_label{nullptr};
    QComboBox *option_list{nullptr};
    QLabel *text_label{nullptr};
    QLineEdit *text_edit{nullptr};
    QPushButton *reset_button{nullptr};
    QPushButton *generate_button{nullptr};
    QTextBrowser *text_browser{nullptr};
    QPushButton *copy_button{nullptr};
    QPushButton *clear_button{nullptr};
};

#endif // COMMAND_H
