#include "gui/gui_application.h"

int main(int argc, char *argv[])
{
    GuiApplication app(argc, argv);

    return app.exec();
}
