#ifndef GUI_TOOL_H
#define GUI_TOOL_H

#include <QWidget>

class GuiTool : public QWidget
{
    Q_OBJECT

public:
    GuiTool() = delete;
    GuiTool(const GuiTool &) = delete;
    virtual ~GuiTool() = default;

protected:
    explicit GuiTool(QWidget *parent = nullptr);

    // NOTE: handle QEvent::LanguageChange to update translation
    virtual void changeEvent(QEvent *event) override = 0;

signals:
};

#endif // GUI_TOOL_H
