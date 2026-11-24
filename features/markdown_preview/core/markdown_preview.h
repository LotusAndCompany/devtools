#ifndef MARKDOWN_PREVIEW_H
#define MARKDOWN_PREVIEW_H

#include "features/framework/core/tool/tool.h"

/**
 * @brief Markdown プレビュー機能のツール識別情報
 */
class MarkdownPreview : public Tool
{
    Q_OBJECT

public:
    explicit MarkdownPreview(QObject *parent = nullptr);
    MarkdownPreview(const MarkdownPreview &) = delete;
    MarkdownPreview(MarkdownPreview &&) = delete;
    MarkdownPreview &operator=(const MarkdownPreview &) = delete;
    MarkdownPreview &operator=(MarkdownPreview &&) = delete;
    ~MarkdownPreview() override = default;
};

#endif // MARKDOWN_PREVIEW_H
