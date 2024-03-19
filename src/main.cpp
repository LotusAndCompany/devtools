#include "gui/gui_application.h"

int main(int argc, char *argv[])
{
    GuiApplication app(argc, argv);
    app.setup();

    return app.start();
}
