#ifndef MARKDOWN_PREVIEW_H
#define MARKDOWN_PREVIEW_H

#include "features/framework/core/tool/tool.h"

#include <QString>

#include <optional>

/**
 * @brief Markdown プレビューツールのロジック
 * @details ファイル I/O と Markdown → HTML 変換のみを担当する。@n
 *          状態を持たないため公開メソッドはすべて `static`。@n
 *          `Tool` を継承するのは ID / 翻訳名を提供するためで、UI には依存しない。
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

    /**
     * @brief テキストファイルを読み込み内容を返す
     * @param path 読み込むファイルのパス
     * @return 成功時はファイル内容(UTF-8)、失敗時は std::nullopt
     */
    static std::optional<QString> readFile(const QString &path);

    /**
     * @brief テキストをファイルへ書き込む
     * @param path 書き込み先パス
     * @param text 書き込む内容
     * @return 成功時 true、失敗時 false
     */
    static bool writeFile(const QString &path, const QString &text);

    /**
     * @brief Markdown を HTML へ変換してファイルへ保存する
     * @details core 内で `QTextDocument` を生成し `toHtml()` を書き出すため、
     *          GUI のウィジェット状態に依存しない。
     * @param path 保存先パス
     * @param markdown Markdown ソース
     * @return 成功時 true、失敗時 false
     */
    static bool exportHtml(const QString &path, const QString &markdown);
};

#endif // MARKDOWN_PREVIEW_H
