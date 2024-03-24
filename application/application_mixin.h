#ifndef APPLICATION_MIXIN_H
#define APPLICATION_MIXIN_H

#include <QCoreApplication>
#include <QTranslator>

class ApplicationMixin
{
    QTranslator _translator;

protected:
    void setupApplication(QCoreApplication *app);
    inline const QTranslator &translator() const { return _translator; }

public:
    ApplicationMixin(int argc, char **argv);
    ApplicationMixin() = delete;
    ~ApplicationMixin() = default;

    virtual void setup() = 0;
    virtual int start() = 0;
};

#endif // APPLICATION_MIXIN_H
