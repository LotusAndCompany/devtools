#ifndef ABOUT_DEVTOOLS_DIALOG_H
#define ABOUT_DEVTOOLS_DIALOG_H

#include <QDialog>

namespace Ui {
class AboutDevToolsDialog;
}

class AboutDevToolsDialog : public QDialog
{
    Q_OBJECT

    Ui::AboutDevToolsDialog *const ui;

public:
    explicit AboutDevToolsDialog(QWidget *parent = nullptr);
    ~AboutDevToolsDialog();
};

#endif // ABOUT_DEVTOOLS_DIALOG_H
