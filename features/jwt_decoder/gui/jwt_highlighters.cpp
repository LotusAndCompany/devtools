#include "jwt_highlighters.h"

#include <QRegularExpressionMatchIterator>
#include <QTextDocument>

namespace {
/// signature 以降はすべて署名として扱うためのセグメント上限
constexpr int LAST_SEGMENT = 2;

QTextCharFormat colorFormat(const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(color);

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

void JwtSegmentHighlighter::setPalette(const JwtPalette &palette)
{
    // NOTE: 無条件の rehighlight() は入力欄の textChanged を発火させるため、
    //       updateState() と同様に色が変わったときだけ呼ぶ。
    if (colors == palette) {
        return;
    }

    colors = palette;

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
        return colorFormat(headerFailed ? colors.errorText : colors.segmentHeader);
    case 1:
        return colorFormat(payloadFailed ? colors.errorText : colors.segmentPayload);
    default:
        return colorFormat(colors.segmentSignature);
    }
}

JsonHighlighter::JsonHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    buildRules();
}

void JsonHighlighter::setPalette(const JwtPalette &palette)
{
    if (colors == palette) {
        return;
    }

    colors = palette;
    buildRules();

    rehighlight();
}

void JsonHighlighter::buildRules()
{
    rules.clear();

    // NOTE: 後のルールが前のルールを上書きするため、数値 → 文字列 → キーの順に並べる。
    //       文字列中の数字がリテラルとして色付けされないようにするための順序。
    rules.append({QRegularExpression(R"(-?\b\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b)"),
                  colorFormat(colors.jsonNumber)});
    rules.append(
        {QRegularExpression(R"(\b(?:true|false|null)\b)"), colorFormat(colors.jsonNumber)});
    rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), colorFormat(colors.jsonString)});
    rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*"(?=\s*:))"), colorFormat(colors.jsonKey)});
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
