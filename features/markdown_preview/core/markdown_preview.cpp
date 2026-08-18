#include "markdown_preview.h"

#include <QFile>
#include <QStringConverter>
#include <QTextDocument>
#include <QTextStream>

MarkdownPreview::MarkdownPreview(QObject *parent)
    : Tool(Tool::ID::MARKDOWN_PREVIEW, "markdown-preview", parent)
{}

std::optional<QString> MarkdownPreview::readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return std::nullopt;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    return stream.readAll();
}

bool MarkdownPreview::writeFile(const QString &path, const QString &text)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << text;

    return stream.status() == QTextStream::Ok;
}

bool MarkdownPreview::exportHtml(const QString &path, const QString &markdown)
{
    QTextDocument document;
    document.setMarkdown(markdown, QTextDocument::MarkdownDialectGitHub);

    return writeFile(path, document.toHtml());
}
