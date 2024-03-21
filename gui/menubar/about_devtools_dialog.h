#ifndef ABOUT_DEVTOOLS_DIALOG_H
#define ABOUT_DEVTOOLS_DIALOG_H

#include <QDialog>

namespace Ui {
class AboutDevToolsDialog;
}

class AboutDevToolsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDevToolsDialog(QWidget *parent = nullptr);
    ~AboutDevToolsDialog();

private:
    Ui::AboutDevToolsDialog *ui;
};

#endif // ABOUT_DEVTOOLS_DIALOG_H
