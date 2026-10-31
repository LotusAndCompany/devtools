#include "markdown_operations.h"

#include "features/markdown_preview/core/markdown_service.h"
#include "operation_support.h"

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

FrontendOperationService::Result FrontendOperationHandlers::renderMarkdown(
    const QVariantMap &payload)
{
    QString markdown;
    if (!readText(payload, QStringLiteral("markdown"), &markdown)) {
        return invalidInput(
            QStringLiteral("Markdown must be a UTF-8 string no larger than 1 MiB."));
    }
    return {{{QStringLiteral("html"), MarkdownService::renderHtml(markdown)}}, {}, {}, false};
}
