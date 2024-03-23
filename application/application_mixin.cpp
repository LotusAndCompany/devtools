#include "application/application_mixin.h"

#include <QDirIterator>
#include <QLocale>
#include <QTranslator>

ApplicationMixin::ApplicationMixin(int argc, char **argv) {}

void ApplicationMixin::setupApplication(QCoreApplication *app)
{
    if (translator.load(":i18n/dev-tools_ja_JP.qm")) {
        qDebug() << "load :i18n/dev-tools_ja_JP.qm";
        app->installTranslator(&translator);
    } else {
        qDebug() << "cannot load :i18n/dev-tools_ja_JP.qm";
    }

    // TODO: 設定ファイル等を読み込む
}
