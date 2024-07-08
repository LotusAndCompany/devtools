#include "color_sample.h"

ColorSample::ColorSample(const QColor &initialColor, QWidget *parent)
    : QWidget(parent)
{
    setColor(initialColor);
}

void ColorSample::setColor(const QColor &) {}
