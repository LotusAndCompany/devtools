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
    /// テキスト内のオフセット位置
    int offset;
    /// キャプチャされた文字列の長さ
    int length;
};

/**
 * @brief 1つのマッチ情報を保持する構造体
 */
struct MatchResult
{
    /// マッチのインデックス
    int index;
    /// テキスト内のオフセット位置
    int offset;
    /// マッチした文字列の長さ
    int length;
    /// キャプチャグループのリスト
    QVector<CaptureGroup> groups;
};

/**
 * @brief 正規表現の処理を行うクラス
 */
class RegexTool
{
public:
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
     * @return 置換後のテキスト
     */
    static QString replace(const QString &pattern, const QString &text, const QString &replacement,
                           QRegularExpression::PatternOptions options);
};

} // namespace devtools

#endif // REGEX_TOOL_H
