#include "core/application_mixin.h"

#include <QDirIterator>
#include <QLocale>
#include <QTranslator>

ApplicationMixin::ApplicationMixin(int argc, char **argv) {}

void ApplicationMixin::setupApplication(QCoreApplication *app)
{
    // TODO: 設定ファイル等を読み込む

    // NOTE: *.uiに未翻訳の環境で表示させる言語で作成するべき?
    // TODO: システムの言語を検知して適切な翻訳ファイルを適用する
    if (_translator.load(":i18n/dev-tools_ja_JP.qm")) {
        qDebug() << "load :i18n/dev-tools_ja_JP.qm";
        app->installTranslator(&_translator);
    } else {
        qDebug() << "cannot load :i18n/dev-tools_ja_JP.qm";
    }
}
