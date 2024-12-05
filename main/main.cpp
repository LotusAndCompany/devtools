#include "gui/gui_application.h"

#if defined(BUILD_TYPE_Debug) || defined(BUILD_TYPE_RelWithDebInfo)
#include <iostream>
__attribute__((destructor)) void destructor(void)
{
    // NOTE: これが実行される時点でメニューバーが解放されていないらしく、メモリリークとして検出されるらしい
    // FIXME: QtCreatorから実行するとファイルへの出力が失敗するるらしい
    const int status = system("leaks -q DevTools &> ./leaks.log");
    if (status) {
        // 標準エラー出力を使うのはやめた方が良いかも
        std::cerr << "Memory leaks are detected!" << std::endl;
        system("cat ./leaks.log >/dev/stderr");
        exit(1);
    }
}
#endif

int main(int argc, char *argv[])
{
    GuiApplication app(argc, argv);
    app.setup();

    return app.start();
}
