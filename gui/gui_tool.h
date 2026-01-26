#ifndef GUI_TOOL_H
#define GUI_TOOL_H

#include <QWidget>

// 0: using GuiTool = QWidget;
// 1: using GuiTool = _GuiToolSuperClass;
#define DevTools_USE_GUI_TOOL_SUPERCLASS 0

#if DevTools_USE_GUI_TOOL_SUPERCLASS
// NOTE: 直接使いたくない名前にする
class _GuiToolSuperClass : public QWidget
{
    Q_OBJECT

public:
    _GuiToolSuperClass() = delete;
    _GuiToolSuperClass(const _GuiToolSuperClass &) = delete;
    virtual ~_GuiToolSuperClass() = default;

protected:
    explicit _GuiToolSuperClass(QWidget *parent = nullptr);

    // NOTE: handle QEvent::LanguageChange to update translation
    virtual void changeEvent(QEvent *event) override = 0;

signals:
};

using GuiTool = _GuiToolSuperClass;
#else
using GuiTool = QWidget;

#if 0
#include <QWidget>
class GuiTool : public QWidget
{
    Q_OBJECT

public:
    GuiTool(QWidget *parent)
        : QWidget(parent)
    {
        qInfo() << "GuiTool::GuiTool";
    }
    virtual ~GuiTool() = default;
};
#endif
#endif

#endif // GUI_TOOL_H
