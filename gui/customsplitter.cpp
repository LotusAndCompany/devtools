#include "customsplitter.h"
#include <QResizeEvent>

CustomSplitter::CustomSplitter(QWidget *parent)
    : QSplitter(parent), minimumHeightMainContent(100)
{
}

void CustomSplitter::resizeEvent(QResizeEvent *event)
{
    QSplitter::resizeEvent(event);
    QList<int> sizes = this->sizes();

    if (sizes[0] < minimumHeightMainContent) {
        sizes[0] = minimumHeightMainContent;
        this->setSizes(sizes);
    }
}
