#ifndef REGEX_TOOL_H
#define REGEX_TOOL_H

#include <QRegularExpression>
#include <QString>
#include <QVector>

namespace devtools {

/**
 * @brief 正規表現のキャプチャグループ情報を保持する構造体
 */
struct CaptureGroup
{
    /// キャプチャされた文字列
    QString value;
    /// キャプチャグループのインデックス
    int index;
    /// 名前付きキャプチャグループの名前（名前がない場合は空文字列）
    QString name;
    /// テキスト内のオフセット位置
    qsizetype offset;
    /// キャプチャされた文字列の長さ
    qsizetype length;
};

/**
 * @brief 1つのマッチ情報を保持する構造体
 */
struct MatchResult
{
    /// マッチのインデックス
    int index;
    /// テキスト内のオフセット位置
    qsizetype offset;
    /// マッチした文字列の長さ
    qsizetype length;
    /// キャプチャグループのリスト
    QVector<CaptureGroup> groups;
};

/**
 * @brief 正規表現の処理を行うクラス
 */
class RegexTool
{
public:
    static QRegularExpression::PatternOptions patternOptions(bool caseInsensitive, bool multiline,
                                                             bool dotMatchesEverything,
                                                             bool useUnicodeProperties,
                                                             bool extendedPatternSyntax);

    /**
     * @brief 正規表現にマッチするか確認し、結果を返す
     * @param pattern 正規表現パターン
     * @param text 対象テキスト
     * @param options オプション
     * @return マッチ結果のリスト
     */
    static QVector<MatchResult> match(const QString &pattern, const QString &text,
                                      QRegularExpression::PatternOptions options);

    /**
     * @brief 正規表現による置換を行う
     * @param pattern 正規表現パターン
     * @param text 対象テキスト
     * @param replacement 置換文字列
     * @param options オプション
     * @param global 全てのマッチを置換するか（falseの場合は最初のマッチのみ）
     * @return 置換後のテキスト
     */
    static QString replace(const QString &pattern, const QString &text, const QString &replacement,
                           QRegularExpression::PatternOptions options, bool global = true);

    /**
     * @brief Build the localized plain-text representation used by Copy Matches
     * @param matches Match results to format
     * @param text Original input text
     * @param headerFormat Localized header format with one `%1` placeholder
     * @param matchFormat Localized match format with `%1` to `%4` placeholders
     * @param groupFormat Localized capture-group format with `%1` to `%4` placeholders
     */
    static QString formatMatchesForClipboard(const QVector<MatchResult> &matches,
                                             const QString &text, const QString &headerFormat,
                                             const QString &matchFormat,
                                             const QString &groupFormat);
};

} // namespace devtools

#endif // REGEX_TOOL_H
