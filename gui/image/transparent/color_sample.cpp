#include "color_sample.h"

#include <QColorDialog>
#include <QPainter>

ColorSample::ColorSample(QWidget *parent)
    : QFrame(parent)
    , _color(QColorConstants::White)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
}

void ColorSample::paintEvent(QPaintEvent *event)
{
    // NOTE: ここで呼んで問題無いか?
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setPen(color());
    painter.fillRect(contentRect(), color());
}

void ColorSample::mousePressEvent(QMouseEvent *event)
{
    QColorDialog dialog;
    dialog.setOption(QColorDialog::ShowAlphaChannel, false);
    dialog.setCurrentColor(color());
    connect(&dialog, &QColorDialog::colorSelected, this, &ColorSample::setColor);
    dialog.exec();
}

void ColorSample::setColor(const QColor &newColor)
{
    if (newColor.alpha() < 255) {
        // 不透明色を設定する
        setColor(QColor(newColor.red(), newColor.green(), newColor.blue()));
    } else {
        _color = newColor;
        emit colorChanged(_color);
        update();
    }
}
