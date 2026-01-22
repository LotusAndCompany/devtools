#ifndef IMAGE_MAIN_H
#define IMAGE_MAIN_H

#include <QWidget>

namespace Ui {
class ImageMain;
}

class ImageMain : public QWidget
{
    Q_OBJECT

public:
    explicit ImageMain(QWidget *parent = nullptr);
    ~ImageMain();

private:
    Ui::ImageMain *ui;
};

#endif // IMAGE_MAIN_H
