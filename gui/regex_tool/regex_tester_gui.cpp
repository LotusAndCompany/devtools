#include "gui/regex_tool/regex_tester_gui.h"

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

namespace devtools {

RegexHighlighter::RegexHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    m_matchFormat.setBackground(Qt::yellow);
    m_matchFormat.setForeground(Qt::black);
}

void RegexHighlighter::setMatches(const QVector<MatchResult> &matches)
{
    m_matches = matches;
    rehighlight();
}

void RegexHighlighter::highlightBlock(const QString &text)
{
    int const blockStart = currentBlock().position();
    int const blockLength = text.length();

    for (const auto &match : m_matches) {
        int const start = match.offset;
        int const end = match.offset + match.length;

        if (end <= blockStart || start >= blockStart + blockLength) {
            continue;
        }

        int const relativeStart = qMax(0, start - blockStart);
        int const relativeLength =
            qMin(blockLength - relativeStart, end - (blockStart + relativeStart));

        setFormat(relativeStart, relativeLength, m_matchFormat);
    }
}

RegexTesterGUI::RegexTesterGUI(QWidget *parent) : GuiTool(parent)
{
    setupUi();
    m_highlighter = new RegexHighlighter(m_testTextEdit->document());
    setupConnections();
    updateResults();
}

RegexTesterGUI::~RegexTesterGUI() = default;

void RegexTesterGUI::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Pattern Row
    auto *patternRow = new QHBoxLayout();
    patternRow->setSpacing(8);

    m_patternEdit = new QLineEdit(this);
    m_patternEdit->setPlaceholderText(tr("Regular Expression Pattern"));
    patternRow->addWidget(m_patternEdit);

    m_flagI = new QToolButton(this);
    m_flagI->setText("i");
    m_flagI->setCheckable(true);
    m_flagI->setToolTip(tr("Case Insensitive"));
    patternRow->addWidget(m_flagI);

    m_flagM = new QToolButton(this);
    m_flagM->setText("m");
    m_flagM->setCheckable(true);
    m_flagM->setToolTip(tr("Multiline"));
    patternRow->addWidget(m_flagM);

    m_flagS = new QToolButton(this);
    m_flagS->setText("s");
    m_flagS->setCheckable(true);
    m_flagS->setToolTip(tr("Dot matches all (Singleline)"));
    patternRow->addWidget(m_flagS);

    m_flagU = new QToolButton(this);
    m_flagU->setText("u");
    m_flagU->setCheckable(true);
    m_flagU->setToolTip(tr("Unicode"));
    patternRow->addWidget(m_flagU);

    mainLayout->addLayout(patternRow);

    // Test Text Label
    mainLayout->addWidget(new QLabel(tr("Test Text"), this));

    // Test Text Edit
    m_testTextEdit = new QPlainTextEdit(this);
    m_testTextEdit->setPlaceholderText(tr("Enter text to test against..."));
    mainLayout->addWidget(m_testTextEdit);

    // Match Result Label
    mainLayout->addWidget(new QLabel(tr("Match Results"), this));

    // Match Result Scroll Area
    auto *matchResultArea = new QScrollArea(this);
    matchResultArea->setWidgetResizable(true);
    auto *matchResultContent = new QWidget();
    m_matchResultLayout = new QVBoxLayout(matchResultContent);
    matchResultArea->setWidget(matchResultContent);
    mainLayout->addWidget(matchResultArea);

    // Replace Row
    auto *replaceRow = new QHBoxLayout();
    replaceRow->setSpacing(8);

    m_replacePatternEdit = new QLineEdit(this);
    m_replacePatternEdit->setPlaceholderText(tr("Replacement String"));
    replaceRow->addWidget(m_replacePatternEdit);

    m_copyResultButton = new QPushButton(tr("Copy Result"), this);
    replaceRow->addWidget(m_copyResultButton);

    mainLayout->addLayout(replaceRow);

    // Replace Result Edit
    m_replaceResultEdit = new QPlainTextEdit(this);
    m_replaceResultEdit->setReadOnly(true);
    m_replaceResultEdit->setPlaceholderText(tr("Replacement result will appear here..."));
    mainLayout->addWidget(m_replaceResultEdit);
}

void RegexTesterGUI::setupConnections()
{
    connect(m_patternEdit, &QLineEdit::textChanged, this, &RegexTesterGUI::updateResults);
    connect(m_flagI, &QToolButton::toggled, this, &RegexTesterGUI::updateResults);
    connect(m_flagM, &QToolButton::toggled, this, &RegexTesterGUI::updateResults);
    connect(m_flagS, &QToolButton::toggled, this, &RegexTesterGUI::updateResults);
    connect(m_flagU, &QToolButton::toggled, this, &RegexTesterGUI::updateResults);
    connect(m_testTextEdit, &QPlainTextEdit::textChanged, this, &RegexTesterGUI::updateResults);
    connect(m_replacePatternEdit, &QLineEdit::textChanged, this, &RegexTesterGUI::updateResults);
    connect(m_copyResultButton, &QPushButton::clicked, this, &RegexTesterGUI::copyToClipboard);
}

QRegularExpression::PatternOptions RegexTesterGUI::currentOptions() const
{
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (m_flagI->isChecked()) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    if (m_flagM->isChecked()) {
        options |= QRegularExpression::MultilineOption;
    }
    if (m_flagS->isChecked()) {
        options |= QRegularExpression::DotMatchesEverythingOption;
    }
    if (m_flagU->isChecked()) {
        options |= QRegularExpression::UseUnicodePropertiesOption;
    }
    return options;
}

void RegexTesterGUI::updateResults()
{
    const QString pattern = m_patternEdit->text();
    const QString text = m_testTextEdit->toPlainText();
    const auto options = currentOptions();

    // マッチング
    const auto matches = RegexTool::match(pattern, text, options);
    m_highlighter->setMatches(matches);
    updateMatchResultDisplay(matches);

    // 置換
    const QString replacement = m_replacePatternEdit->text();
    const QString replacedText = RegexTool::replace(pattern, text, replacement, options);
    m_replaceResultEdit->setPlainText(replacedText);
}

void RegexTesterGUI::updateMatchResultDisplay(const QVector<MatchResult> &matches)
{
    // 既存の結果をクリア
    QLayoutItem *item;
    while ((item = m_matchResultLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    if (matches.isEmpty()) {
        m_matchResultLayout->addWidget(new QLabel(tr("No matches"), this));
    } else {
        for (const auto &match : matches) {
            QString resultText =
                QString("Match %1: [%2, %3] \"%4\"")
                    .arg(match.index + 1)
                    .arg(match.offset)
                    .arg(match.offset + match.length)
                    .arg(m_testTextEdit->toPlainText().mid(match.offset, match.length));

            auto *label = new QLabel(resultText, this);
            label->setWordWrap(true);
            label->setStyleSheet(
                "font-family: monospace; border-bottom: 1px solid #ccc; padding: 4px;");
            m_matchResultLayout->addWidget(label);

            for (int i = 1; i < match.groups.size(); ++i) {
                const auto &group = match.groups[i];
                QString groupText = QString("  Group %1: [%2, %3] \"%4\"")
                                        .arg(group.index)
                                        .arg(group.offset)
                                        .arg(group.offset + group.length)
                                        .arg(group.value);
                auto *groupLabel = new QLabel(groupText, this);
                groupLabel->setStyleSheet(
                    "font-family: monospace; color: #666; padding-left: 16px;");
                m_matchResultLayout->addWidget(groupLabel);
            }
        }
    }
    m_matchResultLayout->addStretch();
}

void RegexTesterGUI::copyToClipboard()
{
    QClipboard *const clipboard = QApplication::clipboard();
    clipboard->setText(m_replaceResultEdit->toPlainText());
}

} // namespace devtools
