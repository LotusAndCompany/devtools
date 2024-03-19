#ifndef APPLICATION_MIXIN_H
#define APPLICATION_MIXIN_H

#include <QCoreApplication>
#include <QTranslator>

class ApplicationMixin
{
    QTranslator translator;

protected:
    void setupApplication(QCoreApplication *app);

public:
    ApplicationMixin(int argc, char **argv);
    ApplicationMixin() = delete;
    ~ApplicationMixin() = default;

    virtual void setup() = 0;
    virtual int exec() = 0;
};

#endif // APPLICATION_MIXIN_H
