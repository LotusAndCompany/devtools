#ifndef HOME_H
#define HOME_H

#include <QWidget>

namespace Ui {
class home;
}

class home : public QWidget
{
    Q_OBJECT

public:
    explicit home(QWidget *parent = nullptr);
    ~home();

private slots:
    // void on_addButton_clicked();

    void on_addButton_clicked();

    void on_showCollectionListButton_clicked();

    void on_closeCollectionListButton_clicked();

private:
    Ui::home *ui;
};

#endif // HOME_H
