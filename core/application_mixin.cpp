#include "application_mixin.h"

#include <QDirIterator>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

ApplicationMixin::ApplicationMixin(AppType appType, int /*argc*/, char ** /*argv*/)
    : appType(appType)
{}

void ApplicationMixin::setupApplication(QCoreApplication *app)
{
    // TODO: 設定ファイル等を読み込む

    // システムロケールを検出
    QString const locale = QLocale::system().name();
    qDebug() << "System locale:" << locale;

    // Qt標準の翻訳ファイルを読み込み（ボタンやダイアログなどの標準UI要素用）
    auto *qtTranslator = new QTranslator(app);
    if (qtTranslator->load("qt_" + locale, QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        qDebug() << "Loaded Qt translations for locale:" << locale;
        QCoreApplication::installTranslator(qtTranslator);
    } else {
        qDebug() << "Could not load Qt translations for locale:" << locale;
        delete qtTranslator;
    }

    // アプリケーション固有の翻訳ファイルを読み込み
    QString const translationFile = QString(":i18n/dev-tools_%1.qm").arg(locale);
    if (_translator.load(translationFile)) {
        qDebug() << "Loaded application translations:" << translationFile;
        QCoreApplication::installTranslator(&_translator);
    } else {
        qDebug() << "Could not load application translations:" << translationFile;
        // 日本語をデフォルトとして試行
        if (locale != "ja_JP" && _translator.load(":i18n/dev-tools_ja_JP.qm")) {
            qDebug() << "Fallback to Japanese translations";
            QCoreApplication::installTranslator(&_translator);
        } else if (locale != "en" && _translator.load(":i18n/dev-tools_en.qm")) {
            qDebug() << "Fallback to English translations";
            QCoreApplication::installTranslator(&_translator);
        }
    }
}
