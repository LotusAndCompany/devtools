#include "jwt_highlighters.h"

#include "features/jwt_decoder/gui/jwt_palette.h"

#include <QColor>
#include <QRegularExpressionMatchIterator>
#include <QTextDocument>

namespace {
/// signature 以降はすべて署名として扱うためのセグメント上限
constexpr int LAST_SEGMENT = 2;

QTextCharFormat colorFormat(const char *color)
{
    QTextCharFormat format;
    format.setForeground(QColor(color));

    return format;
}
} // namespace

JwtSegmentHighlighter::JwtSegmentHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {}

void JwtSegmentHighlighter::updateState(bool enabled, bool header, bool payload)
{
    if (coloringEnabled == enabled && headerFailed == header && payloadFailed == payload) {
        return;
    }

    coloringEnabled = enabled;
    headerFailed = header;
    payloadFailed = payload;

    rehighlight();
}

void JwtSegmentHighlighter::highlightBlock(const QString &text)
{
    if (!coloringEnabled) {
        setCurrentBlockState(0);
        return;
    }

    // 複数行に貼り付けられた場合に備え、直前のブロックのセグメント位置を引き継ぐ
    int segment = qMax(previousBlockState(), 0);
    int start = 0;

    const int length = static_cast<int>(text.size());

    for (int i = 0; i < length; ++i) {
        if (text.at(i) != '.') {
            continue;
        }

        setFormat(start, i - start, segmentFormat(segment));
        start = i + 1;
        segment = qMin(segment + 1, LAST_SEGMENT);
    }

    setFormat(start, length - start, segmentFormat(segment));
    setCurrentBlockState(segment);
}

QTextCharFormat JwtSegmentHighlighter::segmentFormat(int segment) const
{
    switch (segment) {
    case 0:
        return colorFormat(headerFailed ? JwtPalette::ERROR_TEXT : JwtPalette::SEGMENT_HEADER);
    case 1:
        return colorFormat(payloadFailed ? JwtPalette::ERROR_TEXT : JwtPalette::SEGMENT_PAYLOAD);
    default:
        return colorFormat(JwtPalette::SEGMENT_SIGNATURE);
    }
}

JsonHighlighter::JsonHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    // NOTE: 後のルールが前のルールを上書きするため、数値 → 文字列 → キーの順に並べる。
    //       文字列中の数字がリテラルとして色付けされないようにするための順序。
    rules.append({QRegularExpression(R"(-?\b\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b)"),
                  colorFormat(JwtPalette::JSON_NUMBER)});
    rules.append(
        {QRegularExpression(R"(\b(?:true|false|null)\b)"), colorFormat(JwtPalette::JSON_NUMBER)});
    rules.append(
        {QRegularExpression(R"("(?:[^"\\]|\\.)*")"), colorFormat(JwtPalette::JSON_STRING)});
    rules.append(
        {QRegularExpression(R"("(?:[^"\\]|\\.)*"(?=\s*:))"), colorFormat(JwtPalette::JSON_KEY)});
}

void JsonHighlighter::highlightBlock(const QString &text)
{
    for (const Rule &rule : rules) {
        QRegularExpressionMatchIterator matches = rule.pattern.globalMatch(text);
        while (matches.hasNext()) {
            const QRegularExpressionMatch match = matches.next();
            setFormat(static_cast<int>(match.capturedStart()),
                      static_cast<int>(match.capturedLength()), rule.format);
        }
    }
}
