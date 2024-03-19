#include "application/application_mixin.h"

#include <QLocale>
#include <QTranslator>

ApplicationMixin::ApplicationMixin(int argc, char **argv) {}

void ApplicationMixin::setupApplication(QCoreApplication *app)
{
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "dev-tools_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app->installTranslator(&translator);
        }
    }

    // TODO: 設定ファイル等を読み込む
}
