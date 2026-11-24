#include "features/phrase_generation/core/template_reader.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

class TestTemplateReader : public QObject
{
    Q_OBJECT

private slots:
    void readsTemplates();
};

void TestTemplateReader::readsTemplates()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString previous = QDir::currentPath();
    QVERIFY(QDir::setCurrent(directory.path()));
    QVERIFY(TemplateReader::filenames().isEmpty());
    QVERIFY(QDir().mkdir("content"));
    QFile file("content/example.txt");
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write(QStringLiteral("題名\n本文\n二行目").toUtf8()), qint64(23));
    file.close();
    QFile ignored("content/ignored.md");
    QVERIFY(ignored.open(QIODevice::WriteOnly));
    ignored.close();

    QCOMPARE(TemplateReader::filenames(), QStringList({QStringLiteral("example.txt")}));
    QString title;
    QCOMPARE(TemplateReader::load("example.txt", &title), QStringLiteral("本文\n二行目"));
    QCOMPARE(title, QStringLiteral("題名"));
    title = QStringLiteral("unchanged");
    QVERIFY(TemplateReader::load("missing.txt", &title).isEmpty());
    QCOMPARE(title, QStringLiteral("unchanged"));
    QVERIFY(QDir::setCurrent(previous));
}

QTEST_GUILESS_MAIN(TestTemplateReader)
#include "test_template_reader.moc"
