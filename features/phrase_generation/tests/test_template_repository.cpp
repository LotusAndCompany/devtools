#include "features/phrase_generation/core/template_reader.h"
#include "features/phrase_generation/core/template_repository.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

class TestTemplateRepository : public QObject
{
    Q_OBJECT

private slots:
    void validationAndChanges();
    void saveUpdateAndRemove();
};

void TestTemplateRepository::validationAndChanges()
{
    QCOMPARE(TemplateRepository::validate({}, "content"), TemplateValidationError::EmptyTitle);
    QCOMPARE(TemplateRepository::validate("title", {}), TemplateValidationError::EmptyContent);
    QCOMPARE(TemplateRepository::validate("title", "content"), TemplateValidationError::None);
    QVERIFY(!TemplateRepository::hasUnsavedChanges({}, {}, {}));
    QVERIFY(TemplateRepository::hasUnsavedChanges({}, "title", {}));
}

void TestTemplateRepository::saveUpdateAndRemove()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString contentDirectory = directory.filePath("content");

    const TemplateSaveResult saved =
        TemplateRepository::save("題名", "本文\n二行目", {}, contentDirectory);
    QCOMPARE(saved.error, TemplateValidationError::None);
    QVERIFY(saved.written);
    QVERIFY(saved.filename.endsWith(".txt"));
    QVERIFY(QFileInfo::exists(QDir(contentDirectory).filePath(saved.filename)));
    QCOMPARE(TemplateReader::filenames(contentDirectory), QStringList({saved.filename}));

    QString title;
    QCOMPARE(TemplateReader::load(saved.filename, &title, contentDirectory),
             QString("本文\n二行目"));
    QCOMPARE(title, QString("題名"));
    QVERIFY(!TemplateRepository::hasUnsavedChanges(saved.filename, title, "本文\n二行目",
                                                   contentDirectory));
    QVERIFY(
        TemplateRepository::hasUnsavedChanges(saved.filename, title, "edited", contentDirectory));

    const TemplateSaveResult invalid =
        TemplateRepository::save("題名", {}, saved.filename, contentDirectory);
    QCOMPARE(invalid.error, TemplateValidationError::EmptyContent);
    QVERIFY(!invalid.written);
    QVERIFY(QFileInfo::exists(QDir(contentDirectory).filePath(saved.filename)));

    const TemplateSaveResult updated =
        TemplateRepository::save("更新後", "新しい本文", saved.filename, contentDirectory);
    QCOMPARE(updated.error, TemplateValidationError::None);
    QVERIFY(updated.written);
    QVERIFY(updated.filename != saved.filename);
    QVERIFY(!QFileInfo::exists(QDir(contentDirectory).filePath(saved.filename)));
    QCOMPARE(TemplateReader::filenames(contentDirectory), QStringList({updated.filename}));
    QCOMPARE(TemplateReader::load(updated.filename, &title, contentDirectory),
             QString("新しい本文"));
    QCOMPARE(title, QString("更新後"));

    QVERIFY(TemplateRepository::remove(updated.filename, contentDirectory));
    QVERIFY(!TemplateRepository::remove(updated.filename, contentDirectory));

    QFile blocker(directory.filePath("not-directory"));
    QVERIFY(blocker.open(QIODevice::WriteOnly));
    blocker.close();
    const TemplateSaveResult failedWrite = TemplateRepository::save(
        "title", "content", {}, directory.filePath("not-directory/templates"));
    QCOMPARE(failedWrite.error, TemplateValidationError::None);
    QVERIFY(!failedWrite.written);
}

QTEST_GUILESS_MAIN(TestTemplateRepository)
#include "test_template_repository.moc"
