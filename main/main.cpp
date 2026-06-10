#include "gui/gui_application.h"

#include <QDateTime>
#include <QFileInfo>

#if defined(BUILD_TYPE_Debug) || defined(BUILD_TYPE_RelWithDebInfo)
#include <iostream>
__attribute__((destructor)) void destructor()
{
    // NOTE:
    // これが実行される時点でメニューバーが解放されていないらしく、メモリリークとして検出されるらしい
    // FIXME: QtCreatorから実行するとファイルへの出力が失敗するるらしい
    const int status = system("leaks -q DevTools &> ./leaks.log");
    if (status != 0) {
        // 標準エラー出力を使うのはやめた方が良いかも
        std::cerr << "Memory leaks are detected!" << '\n';
        system("cat ./leaks.log >/dev/stderr");
        exit(1);
    }
}
#endif

namespace {
constexpr const char *ANSI_RESET = "\033[0m";
constexpr const char *ANSI_DIM = "\033[2m";
constexpr const char *ANSI_YELLOW = "\033[33m";
constexpr const char *ANSI_RED = "\033[31m";

/// Returns a fixed-width severity label for terminal output.
/// - DEBUG / INFO  (4 chars + trailing space)
/// - WARN  / CRIT  (4 chars + trailing space)
/// - FATAL         (5 chars, no trailing space)
/// - "?????"       (fallback for unknown types)
const char *typeLabel(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return "DEBUG";
    case QtInfoMsg:
        return "INFO ";
    case QtWarningMsg:
        return "WARN ";
    case QtCriticalMsg:
        return "CRIT ";
    case QtFatalMsg:
        return "FATAL";
    }
    return "?????";
}

/// Returns the ANSI escape sequence for the message type's color.
/// - Debug:  dim     (\033[2m)
/// - Info:   default (\033[0m)
/// - Warning: yellow  (\033[33m)
/// - Critical/Fatal: red (\033[31m)
const char *typeColor(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return ANSI_DIM;
    case QtInfoMsg:
        return ANSI_RESET;
    case QtWarningMsg:
        return ANSI_YELLOW;
    case QtCriticalMsg:
    case QtFatalMsg:
        return ANSI_RED;
    }
    return ANSI_RESET;
}

/// Custom Qt message handler that outputs colored, timestamped logs to stderr.
///
/// Output format: {color}[HH:MM:SS.zzz] file.cpp:line TYPE{reset} message
///
/// - file.cpp:line is omitted when source location is unavailable
/// - ANSI color codes are applied per severity
/// - QtFatalMsg calls abort() after printing
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    const QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    const QFileInfo file(context.file != nullptr ? context.file : "");
    const QString location = context.file != nullptr
                                 ? QString("%1:%2").arg(file.fileName()).arg(context.line)
                                 : QString();
    fprintf(stderr, "%s[%s] %s%s%s %s\n", typeColor(type), qPrintable(time),
            qPrintable(location.isEmpty() ? QString() : location + " "), typeLabel(type),
            ANSI_RESET, qPrintable(msg));

    if (type == QtFatalMsg) {
        abort();
    }
}
} // namespace

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    GuiApplication app(argc, argv);
    app.setup();

    return app.start();
}
