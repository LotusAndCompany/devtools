#ifndef DIFF_TOOL_H
#define DIFF_TOOL_H

#include "features/framework/core/tool/tool.h"

#include <QString>
#include <QStringList>
#include <QVector>

/**
 * @brief 差分の1行に対する操作種別
 */
enum class DiffOperation {
    Equal,  ///< 両方のテキストに存在する行
    Delete, ///< 左テキストにのみ存在する行（削除）
    Insert, ///< 右テキストにのみ存在する行（追加）
};

/**
 * @brief 差分の1行分の情報
 */
struct DiffLine
{
    DiffOperation operation; ///< 行の操作種別
    int left_line_number; ///< 左テキストの1始まりの行番号（左に行が無い場合は0）
    int right_line_number; ///< 右テキストの1始まりの行番号（右に行が無い場合は0）
    QString text;          ///< 行の内容
};

/**
 * @brief サイドバイサイド表示のための1行分（左右を対応付けたもの）
 */
struct SideBySideRow
{
    QString left_text;  ///< 左側に表示する行（無い場合は空文字列）
    QString right_text; ///< 右側に表示する行（無い場合は空文字列）
    bool left_changed;  ///< 左側が削除行かどうか
    bool right_changed; ///< 右側が追加行かどうか
};

/**
 * @brief テキスト差分ツールのロジック
 * @details 2つのテキストの行単位の差分を計算する。@n
 *          状態を持たないため公開メソッドはすべて `static`。@n
 *          `Tool` を継承するのは ID / 翻訳名を提供するためで、UI には依存しない。
 */
class DiffTool : public Tool
{
    Q_OBJECT

public:
    explicit DiffTool(QObject *parent = nullptr);
    DiffTool(const DiffTool &) = delete;
    DiffTool(DiffTool &&) = delete;
    DiffTool &operator=(const DiffTool &) = delete;
    DiffTool &operator=(DiffTool &&) = delete;
    ~DiffTool() override = default;

    /**
     * @brief テキストを行のリストへ分割する
     * @details 改行コード（LF / CRLF / CR）を正規化し、末尾の改行による空行は取り除く
     * @param text 分割するテキスト
     * @return 行のリスト
     */
    static QStringList splitLines(const QString &text);

    /**
     * @brief 2つの行リストの差分を計算する
     * @details LCS（最長共通部分列）に基づく行単位の差分を返す
     * @param left 左（変更前）の行リスト
     * @param right 右（変更後）の行リスト
     * @return 差分の行のリスト
     */
    static QVector<DiffLine> diffLines(const QStringList &left, const QStringList &right);

    /**
     * @brief 2つのテキストの差分を計算する
     * @param left_text 左（変更前）のテキスト
     * @param right_text 右（変更後）のテキスト
     * @return 差分の行のリスト
     */
    static QVector<DiffLine> diff(const QString &left_text, const QString &right_text);

    /**
     * @brief サイドバイサイド表示用に左右を対応付ける
     * @details 隣接する「削除 + 追加」を1つの「変更」行にまとめる
     * @param lines 差分の行のリスト
     * @return 左右を対応付けた行のリスト
     */
    static QVector<SideBySideRow> groupSideBySide(const QVector<DiffLine> &lines);
};

#endif // DIFF_TOOL_H
