#ifndef APPLICATION_MIXIN_H
#define APPLICATION_MIXIN_H

#include <QCoreApplication>
#include <QTranslator>

enum class AppType {
    MIN,
    GUI,
    CLI,
    MAX,
};

class ApplicationMixin
{
    QTranslator _translator;

protected:
    void setupApplication(QCoreApplication *app);
    inline const QTranslator &translator() const { return _translator; }

public:
    ApplicationMixin(AppType appType, int argc, char **argv);
    ApplicationMixin() = delete;

    const AppType appType;
    inline bool isGui() const { return appType == AppType::GUI; }
    inline bool isCli() const { return appType == AppType::CLI; }

    virtual void setup() = 0;
    virtual int start() = 0;
};

#endif // APPLICATION_MIXIN_H
