#ifndef API_TOOL_H
#define API_TOOL_H

#include <QFrame>

namespace Ui {
class api_tool;
}

class api_tool : public QFrame
{
    Q_OBJECT
    QWidget *currentContent = nullptr;

public:
    explicit api_tool(QWidget *parent = nullptr);
    ~api_tool();

private:
    Ui::api_tool *ui;
};

#endif // API_TOOL_H
