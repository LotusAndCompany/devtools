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
    if (loadQtTranslator(app, locale)) {
        qDebug() << "Loaded Qt translations for locale:" << locale;
    } else {
        qDebug() << "Could not load Qt translations for locale:" << locale;
    }

    // アプリケーション固有の翻訳ファイルを読み込み
    QString const translationFile = QString(":i18n/dev-tools_%1.qm").arg(locale);
    if (_translator.load(translationFile)) {
        qDebug() << "Loaded application translations:" << translationFile;
        QCoreApplication::installTranslator(&_translator);
    } else {
        qDebug() << "Could not load application translations:" << translationFile;
        qDebug() << "Fallback to source language: English";
    }
}

bool ApplicationMixin::loadQtTranslator(QCoreApplication *app, const QString &languageCode)
{
    if (_qt_translator == nullptr) {
        _qt_translator = new QTranslator(app);
    }

    QCoreApplication::removeTranslator(_qt_translator);

    if (_qt_translator->load("qt_" + languageCode,
                             QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QCoreApplication::installTranslator(_qt_translator);
        return true;
    }

    return false;
}
