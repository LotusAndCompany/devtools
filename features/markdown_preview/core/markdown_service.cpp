#include "markdown_service.h"

#include <QFile>
#include <QStringConverter>
#include <QTextDocument>
#include <QTextStream>

std::optional<QString> MarkdownService::readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return std::nullopt;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    return stream.readAll();
}

bool MarkdownService::writeFile(const QString &path, const QString &text)
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

QString MarkdownService::renderHtml(const QString &markdown)
{
    QTextDocument document;
    document.setMarkdown(markdown, QTextDocument::MarkdownDialectGitHub);
    return document.toHtml();
}

bool MarkdownService::exportHtml(const QString &path, const QString &markdown)
{
    return writeFile(path, renderHtml(markdown));
}
