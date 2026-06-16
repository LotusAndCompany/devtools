#include "gui_tool.h"

#include <QEvent>

#if DevTools_USE_GUI_TOOL_SUPERCLASS
_GuiToolSuperClass::_GuiToolSuperClass(QWidget *parent) : QWidget(parent) {}

void _GuiToolSuperClass::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
}
#endif
