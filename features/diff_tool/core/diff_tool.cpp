#include "diff_tool.h"

#include <vector>

DiffTool::DiffTool(QObject *parent) : Tool(Tool::ID::DIFF_TOOL, "diff-tool", parent) {}

QStringList DiffTool::splitLines(const QString &text)
{
    QString normalized = text;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    QStringList lines = normalized.split(QLatin1Char('\n'));
    if (!lines.isEmpty() && lines.constLast().isEmpty()) {
        lines.removeLast();
    }
    return lines;
}

QVector<DiffLine> DiffTool::diffLines(const QStringList &left, const QStringList &right)
{
    const int n = static_cast<int>(left.size());
    const int m = static_cast<int>(right.size());

    // LCS（最長共通部分列）テーブル。dp(i, j) = left[i..n) と right[j..m) のLCS長。
    // メモリ効率のためフラットな一次元配列に格納する。
    // NOTE: O(n*m) の時間・メモリを要するため、巨大な入力では低速になる。
    const int stride = m + 1;
    std::vector<int> table(static_cast<size_t>(n + 1) * static_cast<size_t>(stride), 0);

    const auto index = [stride](int i, int j) {
        return (i * stride) + j;
    };

    for (int i = n - 1; i >= 0; --i) {
        for (int j = m - 1; j >= 0; --j) {
            if (left.at(i) == right.at(j)) {
                table.at(index(i, j)) = table.at(index(i + 1, j + 1)) + 1;
            } else {
                table.at(index(i, j)) = qMax(table.at(index(i + 1, j)), table.at(index(i, j + 1)));
            }
        }
    }

    // バックトラックして差分の行を構築する
    QVector<DiffLine> result;
    int i = 0;
    int j = 0;
    while (i < n && j < m) {
        if (left.at(i) == right.at(j)) {
            result.append({DiffOperation::Equal, i + 1, j + 1, left.at(i)});
            ++i;
            ++j;
        } else if (table.at(index(i + 1, j)) >= table.at(index(i, j + 1))) {
            result.append({DiffOperation::Delete, i + 1, 0, left.at(i)});
            ++i;
        } else {
            result.append({DiffOperation::Insert, 0, j + 1, right.at(j)});
            ++j;
        }
    }
    while (i < n) {
        result.append({DiffOperation::Delete, i + 1, 0, left.at(i)});
        ++i;
    }
    while (j < m) {
        result.append({DiffOperation::Insert, 0, j + 1, right.at(j)});
        ++j;
    }

    return result;
}

QVector<DiffLine> DiffTool::diff(const QString &left_text, const QString &right_text)
{
    return diffLines(splitLines(left_text), splitLines(right_text));
}

QVector<SideBySideRow> DiffTool::groupSideBySide(const QVector<DiffLine> &lines)
{
    QVector<SideBySideRow> rows;
    const int count = static_cast<int>(lines.size());
    int i = 0;
    while (i < count) {
        const DiffLine &line = lines.at(i);
        if (line.operation == DiffOperation::Equal) {
            rows.append({line.text, line.text, false, false});
            ++i;
        } else if (line.operation == DiffOperation::Delete) {
            if (i + 1 < count && lines.at(i + 1).operation == DiffOperation::Insert) {
                rows.append({line.text, lines.at(i + 1).text, true, true});
                i += 2;
            } else {
                rows.append({line.text, QString(), true, false});
                ++i;
            }
        } else {
            rows.append({QString(), line.text, false, true});
            ++i;
        }
    }
    return rows;
}
