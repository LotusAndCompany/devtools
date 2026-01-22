#ifndef FUNCTION_AREA_H
#define FUNCTION_AREA_H

#include <QWidget>

namespace Ui {
class BasicImageFunctionArea;
}

class BasicImageFunctionArea : public QWidget
{
    Q_OBJECT

public:
    explicit BasicImageFunctionArea(QWidget *parent = nullptr);
    ~BasicImageFunctionArea();

private:
    Ui::BasicImageFunctionArea *ui;
};

#endif // FUNCTION_AREA_H
