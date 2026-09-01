#ifndef JWT_HIGHLIGHTERS_H
#define JWT_HIGHLIGHTERS_H

#include <QList>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

/**
 * @brief 入力欄の JWT を header / payload / signature ごとに色分けする
 * @details `.` 区切りのセグメント位置だけで色を決めるため、デコード結果には依存しない。@n
 *          形式が不正な入力では色分けを無効化して既定色のまま表示する。
 */
class JwtSegmentHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit JwtSegmentHighlighter(QTextDocument *parent = nullptr);

    /**
     * @brief 色分けの状態を更新する
     * @details 状態が変化した場合のみ再ハイライトする。@n
     *          `setFormat()` は入力欄の `textChanged` を発火させるため、無条件に
     *          再ハイライトするとデコード → 再ハイライト → デコードのループになる。
     * @param enabled 形式が正しく色分けを行う場合に true
     * @param header ヘッダーのデコードに失敗したら true
     * @param payload ペイロードのデコードに失敗したら true
     */
    void updateState(bool enabled, bool header, bool payload);

protected:
    void highlightBlock(const QString &text) override;

private:
    [[nodiscard]] QTextCharFormat segmentFormat(int segment) const;

    bool coloringEnabled = true;
    bool headerFailed = false;
    bool payloadFailed = false;
};

/**
 * @brief 整形済み JSON のキー・文字列・数値を色分けする
 */
class JsonHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit JsonHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    /// 適用順に並べた強調ルール (後のルールが前のルールを上書きする)
    struct Rule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QList<Rule> rules;
};

#endif // JWT_HIGHLIGHTERS_H
