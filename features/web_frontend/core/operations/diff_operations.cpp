#include "diff_operations.h"

#include "features/diff_tool/core/diff_tool.h"
#include "operation_support.h"

#include <QMetaType>
#include <QVariantList>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {

FrontendOperationService::Result compareText(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress)
{
    QString left;
    QString right;
    if (!readText(payload, QStringLiteral("left"), &left) ||
        !readText(payload, QStringLiteral("right"), &right)) {
        return invalidInput(
            QStringLiteral("Both texts must be UTF-8 strings no larger than 1 MiB."));
    }
    if (DiffTool::splitLines(left).size() > 2000 || DiffTool::splitLines(right).size() > 2000) {
        return {{},
                QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                QStringLiteral("Diff is limited to 2,000 lines per side."),
                false};
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }
    progress(20, QStringLiteral("Comparing text"));
    const QVector<SideBySideRow> rows = DiffTool::groupSideBySide(DiffTool::diff(left, right));
    QVariantList output;
    output.reserve(rows.size());
    for (const SideBySideRow &row : rows) {
        if (cancellation->load(std::memory_order_relaxed)) {
            return {{}, {}, {}, true};
        }
        output.append(QVariantMap{{QStringLiteral("left"), row.left_text},
                                  {QStringLiteral("right"), row.right_text},
                                  {QStringLiteral("leftChanged"), row.left_changed},
                                  {QStringLiteral("rightChanged"), row.right_changed}});
    }
    return {{{QStringLiteral("rows"), output}}, {}, {}, false};
}

} // namespace FrontendOperationHandlers
