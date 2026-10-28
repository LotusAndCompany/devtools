#include "react_bridge_demo_widget.h"

#include "qt_webengine_adapter.h"

#include <QVBoxLayout>

ReactBridgeDemoWidget::ReactBridgeDemoWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(new QtWebEngineAdapter(this));
}
