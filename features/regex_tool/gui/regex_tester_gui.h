#ifndef REGEX_TESTER_GUI_H
#define REGEX_TESTER_GUI_H

#include "features/framework/gui/gui_tool.h"
#include "features/regex_tool/core/regex_tool.h"

#include <QMutex>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QThread>
#include <QTimer>

class QLineEdit;
class QToolButton;
class QPlainTextEdit;
class QPushButton;
class QVBoxLayout;
class QComboBox;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;

namespace devtools {

/**
 * @brief Background worker to evaluate regular expressions asynchronously
 */
class RegexWorker : public QThread
{
    Q_OBJECT
public:
    explicit RegexWorker(QObject *parent = nullptr);
    void setParams(int requestId, const QString &pattern, const QString &text,
                   const QString &replacePattern, QRegularExpression::PatternOptions options,
                   bool global);

signals:
    void finishedMatching(int requestId, const QVector<MatchResult> &matches,
                          const QString &replacedText, bool isValid, const QString &errorStr);

protected:
    void run() override;

private:
    QMutex m_mutex;
    int m_requestId;
    QString m_pattern;
    QString m_text;
    QString m_replacePattern;
    QRegularExpression::PatternOptions m_options;
    bool m_global;
};

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
    void triggerUpdate();
    void updateResults();
    void onMatchingFinished(int requestId, const QVector<MatchResult> &matches,
                            const QString &replacedText, bool isValid, const QString &errorStr);
    void onWatchdogTimeout();
    void copyPattern();
    void copyMatches();
    void copyToClipboard();
    void onPresetSelected(int index);
    void toggleQuickRef();
    void onQuickRefDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void setupUi();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    QRegularExpression::PatternOptions currentOptions() const;
    void updateMatchResultDisplay(const QVector<MatchResult> &matches);

    // UI Components
    QLineEdit *m_patternEdit = nullptr;
    QPushButton *m_copyPatternButton = nullptr;
    QComboBox *m_presetCombo = nullptr;

    QToolButton *m_flagG = nullptr;
    QToolButton *m_flagI = nullptr;
    QToolButton *m_flagM = nullptr;
    QToolButton *m_flagS = nullptr;
    QToolButton *m_flagU = nullptr;
    QToolButton *m_flagX = nullptr;

    QLabel *m_errorLabel = nullptr;

    QPlainTextEdit *m_testTextEdit = nullptr;

    QLabel *m_matchCountLabel = nullptr;
    QPushButton *m_copyMatchesButton = nullptr;
    QVBoxLayout *m_matchResultLayout = nullptr;

    QLineEdit *m_replacePatternEdit = nullptr;
    QPushButton *m_copyResultButton = nullptr;
    QPlainTextEdit *m_replaceResultEdit = nullptr;

    QPushButton *m_quickRefToggle = nullptr;
    QWidget *m_quickRefContainer = nullptr;
    QTreeWidget *m_quickRefTree = nullptr;

    RegexHighlighter *m_highlighter = nullptr;
    RegexWorker *m_worker = nullptr;
    bool m_updatePending = false;
    QTimer *m_debounceTimer = nullptr;
    QTimer *m_watchdogTimer = nullptr;
    QVector<MatchResult> m_lastMatches;
    int m_requestId = 0;
};

} // namespace devtools

#endif // REGEX_TESTER_GUI_H
