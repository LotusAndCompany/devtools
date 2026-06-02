#ifndef DEVTOOLS_GUI_REGEX_TOOL_REGEX_TESTER_GUI_H
#define DEVTOOLS_GUI_REGEX_TOOL_REGEX_TESTER_GUI_H

#include "core/regex_tool/regex_tool.h"
#include "gui/gui_tool.h"

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QLineEdit;
class QToolButton;
class QPlainTextEdit;
class QPushButton;
class QVBoxLayout;

namespace devtools {

/**
 * @brief 正規表現のハイライトを行うクラス
 */
class RegexHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit RegexHighlighter(QTextDocument *parent = nullptr);
    void setMatches(const QVector<MatchResult> &matches);

protected:
    void highlightBlock(const QString &text) override;

private:
    QVector<MatchResult> m_matches;
    QTextCharFormat m_matchFormat;
};

/**
 * @brief 正規表現テスターのUIクラス (C++でレイアウトを作成)
 */
class RegexTesterGUI : public GuiTool
{
    Q_OBJECT

public:
    explicit RegexTesterGUI(QWidget *parent = nullptr);
    ~RegexTesterGUI() override;

private slots:
    void updateResults();
    void copyToClipboard();

private:
    void setupUi();
    void setupConnections();
    QRegularExpression::PatternOptions currentOptions() const;
    void updateMatchResultDisplay(const QVector<MatchResult> &matches);

    // UI Components
    QLineEdit *m_patternEdit = nullptr;
    QToolButton *m_flagI = nullptr;
    QToolButton *m_flagM = nullptr;
    QToolButton *m_flagS = nullptr;
    QToolButton *m_flagU = nullptr;
    QPlainTextEdit *m_testTextEdit = nullptr;
    QVBoxLayout *m_matchResultLayout = nullptr;
    QLineEdit *m_replacePatternEdit = nullptr;
    QPushButton *m_copyResultButton = nullptr;
    QPlainTextEdit *m_replaceResultEdit = nullptr;

    RegexHighlighter *m_highlighter = nullptr;
};

} // namespace devtools

#endif // DEVTOOLS_GUI_REGEX_TOOL_REGEX_TESTER_GUI_H
