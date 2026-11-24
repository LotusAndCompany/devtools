#include "features/markdown_preview/core/markdown_preview.h"
#include "features/markdown_preview/core/markdown_service.h"
#include "features/markdown_preview/gui/markdown_preview_gui.h"

#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QTextDocument>
#include <QtTest>

class TestMarkdownPreviewGui : public QObject
{
    Q_OBJECT

private slots:
    void rendersThroughCoreService();
};

void TestMarkdownPreviewGui::rendersThroughCoreService()
{
    auto *tool = new MarkdownPreview;
    MarkdownPreviewGUI widget(tool);
    auto *editor = widget.findChild<QPlainTextEdit *>("markdownEditor");
    auto *preview = widget.findChild<QTextBrowser *>("markdownPreview");
    QVERIFY(editor != nullptr);
    QVERIFY(preview != nullptr);

    const QString markdown = "# Heading\n\n**strong**";
    editor->setPlainText(markdown);
    QTRY_VERIFY_WITH_TIMEOUT(preview->toPlainText().contains("Heading"), 2000);
    QCOMPARE(preview->toPlainText(), QString("Heading\nstrong"));
    QTextDocument baseline;
    baseline.setMarkdown(markdown, QTextDocument::MarkdownDialectGitHub);
    QCOMPARE(preview->toPlainText(), baseline.toPlainText());
    QVERIFY(preview->document()->toHtml().contains("<h1"));
    QVERIFY(preview->document()->toHtml().contains("Heading"));
    QVERIFY(preview->document()->toHtml().contains("strong"));
    QVERIFY(MarkdownService::renderHtml(markdown).contains("<h1"));
}

QTEST_MAIN(TestMarkdownPreviewGui)
#include "test_markdown_preview_gui.moc"
