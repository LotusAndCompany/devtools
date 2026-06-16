#include "core/markdown_preview/markdown_preview.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest>

namespace Test {
class TestMarkdownPreview : public QObject
{
    Q_OBJECT

private slots:
    // Test cases:
    static void test_readFile();
    static void test_readFile_notFound();
    static void test_writeFile();
    static void test_exportHtml();
};

void TestMarkdownPreview::test_readFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath("sample.md");
    const QString content = "# Title\n\nbody text\n";
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream stream(&file);
        stream << content;
    }

    const std::optional<QString> result = MarkdownPreview::readFile(path);
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), content);
}

void TestMarkdownPreview::test_readFile_notFound()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const std::optional<QString> result = MarkdownPreview::readFile(dir.filePath("missing.md"));
    QVERIFY(!result.has_value());
}

void TestMarkdownPreview::test_writeFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath("out.md");
    const QString content = "- item 1\n- item 2\n";

    QVERIFY(MarkdownPreview::writeFile(path, content));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream stream(&file);
    QCOMPARE(stream.readAll(), content);
}

void TestMarkdownPreview::test_exportHtml()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath("out.html");

    QVERIFY(MarkdownPreview::exportHtml(path, "# hi"));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString html = QTextStream(&file).readAll();
    QVERIFY(html.contains("<html", Qt::CaseInsensitive));
    QVERIFY(html.contains("hi"));
}
} // namespace Test

// QTextDocument は QImage 系テストと同様に QApplication 不要で動作する
QTEST_APPLESS_MAIN(Test::TestMarkdownPreview)

#include "test_markdown_preview.moc"
