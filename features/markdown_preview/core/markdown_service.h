#ifndef MARKDOWN_SERVICE_H
#define MARKDOWN_SERVICE_H

#include <QString>

#include <optional>

class MarkdownService
{
public:
    static std::optional<QString> readFile(const QString &path);
    static bool writeFile(const QString &path, const QString &text);
    static QString renderHtml(const QString &markdown);
    static bool exportHtml(const QString &path, const QString &markdown);
};

#endif // MARKDOWN_SERVICE_H
