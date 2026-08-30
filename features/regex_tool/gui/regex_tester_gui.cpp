#include "features/regex_tool/gui/regex_tester_gui.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMutexLocker>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSplitter>
#include <QTextStream>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace devtools {

// ============================================================================
// RegexWorker Implementation
// ============================================================================

RegexWorker::RegexWorker(QObject *parent)
    : QThread(parent), m_options(QRegularExpression::NoPatternOption), m_global(true)
{}

void RegexWorker::setParams(const QString &pattern, const QString &text,
                            const QString &replacePattern,
                            QRegularExpression::PatternOptions options, bool global)
{
    QMutexLocker locker(&m_mutex);
    m_pattern = pattern;
    m_text = text;
    m_replacePattern = replacePattern;
    m_options = options;
    m_global = global;
}

void RegexWorker::run()
{
    QString pattern;
    QString text;
    QString replacePattern;
    QRegularExpression::PatternOptions options;
    bool global;
    {
        QMutexLocker locker(&m_mutex);
        pattern = m_pattern;
        text = m_text;
        replacePattern = m_replacePattern;
        options = m_options;
        global = m_global;
    }

    QVector<MatchResult> matches;
    if (pattern.isEmpty()) {
        emit finishedMatching(matches, text, true, QString());
        return;
    }

    QRegularExpression const re(pattern, options);
    if (!re.isValid()) {
        emit finishedMatching(matches, text, false, re.errorString());
        return;
    }

    // Execute match
    if (global) {
        QRegularExpressionMatchIterator i = re.globalMatch(text);
        int match_index = 0;
        while (i.hasNext()) {
            if (isInterruptionRequested())
                return;
            QRegularExpressionMatch const match = i.next();
            MatchResult res;
            res.index = match_index++;
            res.offset = match.capturedStart();
            res.length = match.capturedLength();

            for (int j = 0; j <= match.lastCapturedIndex(); ++j) {
                CaptureGroup group;
                group.index = j;
                group.value = match.captured(j);
                group.offset = match.capturedStart(j);
                group.length = match.capturedLength(j);
                res.groups.append(group);
            }
            matches.append(res);
        }
    } else {
        QRegularExpressionMatch match = re.match(text);
        if (match.hasMatch()) {
            MatchResult res;
            res.index = 0;
            res.offset = match.capturedStart();
            res.length = match.capturedLength();

            for (int j = 0; j <= match.lastCapturedIndex(); ++j) {
                CaptureGroup group;
                group.index = j;
                group.value = match.captured(j);
                group.offset = match.capturedStart(j);
                group.length = match.capturedLength(j);
                res.groups.append(group);
            }
            matches.append(res);
        }
    }

    if (isInterruptionRequested())
        return;

    // Execute replace using custom ECMA-like substitution
    QString replacedText = RegexTool::replace(pattern, text, replacePattern, options);

    emit finishedMatching(matches, replacedText, true, QString());
}

// ============================================================================
// RegexHighlighter Implementation
// ============================================================================

RegexHighlighter::RegexHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    // Semi-transparent yellow background that works nicely in light & dark modes
    m_matchFormat.setBackground(QBrush(QColor(255, 220, 0, 75)));
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

// ============================================================================
// RegexTesterGUI Implementation
// ============================================================================

RegexTesterGUI::RegexTesterGUI(QWidget *parent) : GuiTool(parent)
{
    m_worker = new RegexWorker(this);
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_watchdogTimer = new QTimer(this);
    m_watchdogTimer->setSingleShot(true);

    setupUi();
    m_highlighter = new RegexHighlighter(m_testTextEdit->document());
    setupConnections();

    loadSettings();
    triggerUpdate();
}

RegexTesterGUI::~RegexTesterGUI()
{
    saveSettings();
    if (m_worker->isRunning()) {
        m_worker->terminate();
        m_worker->wait();
    }
}

void RegexTesterGUI::setupUi()
{
    auto *const mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    auto *const splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(splitter);

    // --------------------------------------------------
    // Left Pane (Inputs & Replaced Output)
    // --------------------------------------------------
    auto *const leftWidget = new QWidget(splitter);
    auto *const leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    // Title Row
    auto *const titleLabel = new QLabel(tr("Regex Tester"), leftWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(16);
    titleLabel->setFont(titleFont);
    leftLayout->addWidget(titleLabel);

    // Presets Row
    auto *const presetsRow = new QHBoxLayout();
    presetsRow->setSpacing(8);
    m_presetCombo = new QComboBox(leftWidget);
    m_presetCombo->addItem(tr("Select preset pattern..."));
    m_presetCombo->addItem(tr("Email Address"));
    m_presetCombo->addItem(tr("URL (Web Address)"));
    m_presetCombo->addItem(tr("IPv4 Address"));
    m_presetCombo->addItem(tr("ISO Date (YYYY-MM-DD)"));
    m_presetCombo->addItem(tr("Phone Number (E.164)"));
    m_presetCombo->addItem(tr("UUID v4"));
    presetsRow->addWidget(m_presetCombo, 1);

    m_copyPatternButton = new QPushButton(tr("Copy Pattern"), leftWidget);
    presetsRow->addWidget(m_copyPatternButton);
    leftLayout->addLayout(presetsRow);

    // Pattern Edit
    m_patternEdit = new QLineEdit(leftWidget);
    m_patternEdit->setPlaceholderText(tr("Regular Expression Pattern"));
    m_patternEdit->setFont(QFont("monospace", 13));
    leftLayout->addWidget(m_patternEdit);

    // Flags Row
    auto *const flagsLayout = new QHBoxLayout();
    flagsLayout->setSpacing(4);

    m_flagG = new QToolButton(leftWidget);
    m_flagG->setText("g");
    m_flagG->setCheckable(true);
    m_flagG->setChecked(true);
    m_flagG->setToolTip(tr("Global (match all)"));
    flagsLayout->addWidget(m_flagG);

    m_flagI = new QToolButton(leftWidget);
    m_flagI->setText("i");
    m_flagI->setCheckable(true);
    m_flagI->setToolTip(tr("Case Insensitive"));
    flagsLayout->addWidget(m_flagI);

    m_flagM = new QToolButton(leftWidget);
    m_flagM->setText("m");
    m_flagM->setCheckable(true);
    m_flagM->setToolTip(tr("Multiline"));
    flagsLayout->addWidget(m_flagM);

    m_flagS = new QToolButton(leftWidget);
    m_flagS->setText("s");
    m_flagS->setCheckable(true);
    m_flagS->setToolTip(tr("Dot matches all (Singleline)"));
    flagsLayout->addWidget(m_flagS);

    m_flagU = new QToolButton(leftWidget);
    m_flagU->setText("u");
    m_flagU->setCheckable(true);
    m_flagU->setToolTip(tr("Unicode"));
    flagsLayout->addWidget(m_flagU);

    m_flagX = new QToolButton(leftWidget);
    m_flagX->setText("x");
    m_flagX->setCheckable(true);
    m_flagX->setToolTip(tr("Extended (ignore whitespace in pattern)"));
    flagsLayout->addWidget(m_flagX);

    flagsLayout->addStretch();
    leftLayout->addLayout(flagsLayout);

    // Error Label
    m_errorLabel = new QLabel(leftWidget);
    m_errorLabel->setStyleSheet("color: red; font-family: monospace; font-size: 11px;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    leftLayout->addWidget(m_errorLabel);

    // Test Text Label
    leftLayout->addWidget(new QLabel(tr("Test Text"), leftWidget));

    // Test Text Edit
    m_testTextEdit = new QPlainTextEdit(leftWidget);
    m_testTextEdit->setPlaceholderText(tr("Enter text to test against..."));
    m_testTextEdit->setFont(QFont("monospace", 13));
    leftLayout->addWidget(m_testTextEdit, 2);

    // Replace Label / Row
    leftLayout->addWidget(new QLabel(tr("Substitution / Replace"), leftWidget));

    auto *const replaceRow = new QHBoxLayout();
    replaceRow->setSpacing(8);
    m_replacePatternEdit = new QLineEdit(leftWidget);
    m_replacePatternEdit->setPlaceholderText(tr("Replacement String"));
    m_replacePatternEdit->setFont(QFont("monospace", 13));
    replaceRow->addWidget(m_replacePatternEdit, 1);

    m_copyResultButton = new QPushButton(tr("Copy Result"), leftWidget);
    replaceRow->addWidget(m_copyResultButton);
    leftLayout->addLayout(replaceRow);

    // Replace Result Edit
    m_replaceResultEdit = new QPlainTextEdit(leftWidget);
    m_replaceResultEdit->setReadOnly(true);
    m_replaceResultEdit->setPlaceholderText(tr("Replacement result will appear here..."));
    m_replaceResultEdit->setFont(QFont("monospace", 13));
    leftLayout->addWidget(m_replaceResultEdit, 1);

    splitter->addWidget(leftWidget);

    // --------------------------------------------------
    // Right Pane (Match Inspector & Quick Reference)
    // --------------------------------------------------
    auto *const rightWidget = new QWidget(splitter);
    auto *const rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    // Matches Header Row
    auto *const matchesHeaderRow = new QHBoxLayout();
    m_matchCountLabel = new QLabel(tr("Matches (0)"), rightWidget);
    QFont boldFont = m_matchCountLabel->font();
    boldFont.setBold(true);
    m_matchCountLabel->setFont(boldFont);
    matchesHeaderRow->addWidget(m_matchCountLabel);

    matchesHeaderRow->addStretch();

    m_copyMatchesButton = new QPushButton(tr("Copy Matches"), rightWidget);
    matchesHeaderRow->addWidget(m_copyMatchesButton);
    rightLayout->addLayout(matchesHeaderRow);

    // Match Result Scroll Area
    auto *const matchResultArea = new QScrollArea(rightWidget);
    matchResultArea->setWidgetResizable(true);
    auto *const matchResultContent = new QWidget();
    m_matchResultLayout = new QVBoxLayout(matchResultContent);
    m_matchResultLayout->setSpacing(4);
    m_matchResultLayout->setContentsMargins(4, 4, 4, 4);
    matchResultArea->setWidget(matchResultContent);
    rightLayout->addWidget(matchResultArea, 2);

    // Quick Reference Collapsible Section
    m_quickRefToggle = new QPushButton(tr("Hide Quick Reference"), rightWidget);
    rightLayout->addWidget(m_quickRefToggle);

    m_quickRefContainer = new QWidget(rightWidget);
    auto *const quickRefLayout = new QVBoxLayout(m_quickRefContainer);
    quickRefLayout->setContentsMargins(0, 0, 0, 0);

    m_quickRefTree = new QTreeWidget(m_quickRefContainer);
    m_quickRefTree->setHeaderHidden(true);
    m_quickRefTree->setColumnCount(2);
    m_quickRefTree->setColumnWidth(0, 110);
    m_quickRefTree->setToolTip(tr("Double-click to insert token into pattern"));

    // Character Classes
    auto *charClasses =
        new QTreeWidgetItem(m_quickRefTree, QStringList() << tr("Character Classes"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\d)" << tr("Digit (0-9)"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\D)" << tr("Non-digit"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\w)" << tr("Word char (a-z, 0-9, _)"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\W)" << tr("Non-word char"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\s)" << tr("Whitespace"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\S)" << tr("Non-whitespace"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(.)" << tr("Any character except newline"));

    // Quantifiers
    auto *quantifiers = new QTreeWidgetItem(m_quickRefTree, QStringList() << tr("Quantifiers"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"(*)" << tr("0 or more times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"(+)" << tr("1 or more times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"(?)" << tr("0 or 1 time (optional)"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"({n})" << tr("Exactly n times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"({n,})" << tr("n or more times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"({n,m})" << tr("Between n and m times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"(*?)" << tr("Lazy quantifier"));

    // Anchors
    auto *anchors = new QTreeWidgetItem(m_quickRefTree, QStringList() << tr("Anchors"));
    new QTreeWidgetItem(anchors, QStringList() << R"(^)" << tr("Start of string / line"));
    new QTreeWidgetItem(anchors, QStringList() << R"($)" << tr("End of string / line"));
    new QTreeWidgetItem(anchors, QStringList() << R"(\b)" << tr("Word boundary"));
    new QTreeWidgetItem(anchors, QStringList() << R"(\B)" << tr("Non-word boundary"));

    // Groups & References
    auto *groups = new QTreeWidgetItem(m_quickRefTree, QStringList() << tr("Groups & References"));
    new QTreeWidgetItem(groups, QStringList() << R"((...))" << tr("Capture group"));
    new QTreeWidgetItem(groups, QStringList() << R"((?:...))" << tr("Non-capturing group"));
    new QTreeWidgetItem(groups, QStringList() << R"((?<name>...))" << tr("Named capture group"));
    new QTreeWidgetItem(groups, QStringList() << R"(\1)" << tr("Match group #1 reference"));
    new QTreeWidgetItem(groups, QStringList()
                                    << R"(\k<name>)" << tr("Match named group reference"));

    m_quickRefTree->expandAll();
    quickRefLayout->addWidget(m_quickRefTree);
    rightLayout->addWidget(m_quickRefContainer, 1);

    splitter->addWidget(rightWidget);
    splitter->setSizes({600, 300});
}

void RegexTesterGUI::setupConnections()
{
    connect(m_patternEdit, &QLineEdit::textChanged, this, &RegexTesterGUI::triggerUpdate);
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &RegexTesterGUI::onPresetSelected);
    connect(m_copyPatternButton, &QPushButton::clicked, this, &RegexTesterGUI::copyPattern);

    connect(m_flagG, &QToolButton::toggled, this, &RegexTesterGUI::triggerUpdate);
    connect(m_flagI, &QToolButton::toggled, this, &RegexTesterGUI::triggerUpdate);
    connect(m_flagM, &QToolButton::toggled, this, &RegexTesterGUI::triggerUpdate);
    connect(m_flagS, &QToolButton::toggled, this, &RegexTesterGUI::triggerUpdate);
    connect(m_flagU, &QToolButton::toggled, this, &RegexTesterGUI::triggerUpdate);
    connect(m_flagX, &QToolButton::toggled, this, &RegexTesterGUI::triggerUpdate);

    connect(m_testTextEdit, &QPlainTextEdit::textChanged, this, &RegexTesterGUI::triggerUpdate);
    connect(m_replacePatternEdit, &QLineEdit::textChanged, this, &RegexTesterGUI::triggerUpdate);

    connect(m_copyMatchesButton, &QPushButton::clicked, this, &RegexTesterGUI::copyMatches);
    connect(m_copyResultButton, &QPushButton::clicked, this, &RegexTesterGUI::copyToClipboard);

    connect(m_quickRefToggle, &QPushButton::clicked, this, &RegexTesterGUI::toggleQuickRef);
    connect(m_quickRefTree, &QTreeWidget::itemDoubleClicked, this,
            &RegexTesterGUI::onQuickRefDoubleClicked);

    // Timer connections
    connect(m_debounceTimer, &QTimer::timeout, this, &RegexTesterGUI::updateResults);
    connect(m_watchdogTimer, &QTimer::timeout, this, &RegexTesterGUI::onWatchdogTimeout);

    // Background worker connection
    connect(m_worker, &RegexWorker::finishedMatching, this, &RegexTesterGUI::onMatchingFinished);
}

void RegexTesterGUI::loadSettings()
{
    QSettings settings;
    m_patternEdit->setText(settings.value("RegexTester/pattern", "").toString());
    m_testTextEdit->setPlainText(settings.value("RegexTester/testText", "").toString());
    m_replacePatternEdit->setText(settings.value("RegexTester/replacePattern", "").toString());

    m_flagG->setChecked(settings.value("RegexTester/flagG", true).toBool());
    m_flagI->setChecked(settings.value("RegexTester/flagI", false).toBool());
    m_flagM->setChecked(settings.value("RegexTester/flagM", false).toBool());
    m_flagS->setChecked(settings.value("RegexTester/flagS", false).toBool());
    m_flagU->setChecked(settings.value("RegexTester/flagU", false).toBool());
    m_flagX->setChecked(settings.value("RegexTester/flagX", false).toBool());
}

void RegexTesterGUI::saveSettings()
{
    QSettings settings;
    settings.setValue("RegexTester/pattern", m_patternEdit->text());
    settings.setValue("RegexTester/testText", m_testTextEdit->toPlainText());
    settings.setValue("RegexTester/replacePattern", m_replacePatternEdit->text());

    settings.setValue("RegexTester/flagG", m_flagG->isChecked());
    settings.setValue("RegexTester/flagI", m_flagI->isChecked());
    settings.setValue("RegexTester/flagM", m_flagM->isChecked());
    settings.setValue("RegexTester/flagS", m_flagS->isChecked());
    settings.setValue("RegexTester/flagU", m_flagU->isChecked());
    settings.setValue("RegexTester/flagX", m_flagX->isChecked());
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
    if (m_flagX->isChecked()) {
        options |= QRegularExpression::ExtendedPatternSyntaxOption;
    }
    return options;
}

void RegexTesterGUI::triggerUpdate()
{
    m_debounceTimer->start(100);
}

void RegexTesterGUI::updateResults()
{
    m_debounceTimer->stop();
    m_watchdogTimer->stop();

    if (m_worker->isRunning()) {
        m_worker->terminate();
        m_worker->wait();
    }

    const QString pattern = m_patternEdit->text();
    const QString text = m_testTextEdit->toPlainText();
    const QString replacePattern = m_replacePatternEdit->text();
    const auto options = currentOptions();
    bool const global = m_flagG->isChecked();

    m_worker->setParams(pattern, text, replacePattern, options, global);
    m_worker->start();
    m_watchdogTimer->start(500); // 500ms limit
}

void RegexTesterGUI::onMatchingFinished(const QVector<MatchResult> &matches,
                                        const QString &replacedText, bool isValid,
                                        const QString &errorStr)
{
    m_watchdogTimer->stop();

    if (!isValid) {
        m_errorLabel->setText(tr("Syntax Error: %1").arg(errorStr));
        m_errorLabel->show();

        m_lastMatches.clear();
        m_highlighter->setMatches(m_lastMatches);
        updateMatchResultDisplay(m_lastMatches);
        m_replaceResultEdit->setPlainText(m_testTextEdit->toPlainText());
    } else {
        m_errorLabel->hide();
        m_errorLabel->setText(QString());

        m_lastMatches = matches;
        m_highlighter->setMatches(m_lastMatches);
        updateMatchResultDisplay(m_lastMatches);
        m_replaceResultEdit->setPlainText(replacedText);
    }
}

void RegexTesterGUI::onWatchdogTimeout()
{
    if (m_worker->isRunning()) {
        m_worker->terminate();
        m_worker->wait();

        m_errorLabel->setText(
            tr("Error: Evaluation timed out (catastrophic backtracking detected)"));
        m_errorLabel->show();

        m_lastMatches.clear();
        m_highlighter->setMatches(m_lastMatches);
        updateMatchResultDisplay(m_lastMatches);
        m_replaceResultEdit->setPlainText(m_testTextEdit->toPlainText());
    }
}

void RegexTesterGUI::updateMatchResultDisplay(const QVector<MatchResult> &matches)
{
    // Clear old result
    QLayoutItem *item;
    while ((item = m_matchResultLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    m_matchCountLabel->setText(tr("Matches (%1)").arg(matches.size()));

    if (matches.isEmpty()) {
        auto *const label = new QLabel(tr("No matches"), this);
        label->setStyleSheet("color: gray;");
        m_matchResultLayout->addWidget(label);
    } else {
        for (const auto &match : matches) {
            QString const matchText = m_testTextEdit->toPlainText().mid(match.offset, match.length);
            QString const resultText = QString("Match %1: [%2, %3]\n\"%4\"")
                                           .arg(match.index + 1)
                                           .arg(match.offset)
                                           .arg(match.offset + match.length)
                                           .arg(matchText);

            auto *const label = new QLabel(resultText, this);
            label->setWordWrap(true);
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            label->setStyleSheet(
                "font-family: monospace; border-bottom: 1px solid #ccc; padding: 4px;");
            m_matchResultLayout->addWidget(label);

            for (int i = 1; i < match.groups.size(); ++i) {
                const auto &group = match.groups[i];
                QString const groupText = QString("  Group %1: [%2, %3]\n  \"%4\"")
                                              .arg(group.index)
                                              .arg(group.offset)
                                              .arg(group.offset + group.length)
                                              .arg(group.value);
                auto *const groupLabel = new QLabel(groupText, this);
                groupLabel->setWordWrap(true);
                groupLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
                groupLabel->setStyleSheet(
                    "font-family: monospace; color: #666; padding-left: 16px; border-bottom: 1px "
                    "dashed #eee;");
                m_matchResultLayout->addWidget(groupLabel);
            }
        }
    }
    m_matchResultLayout->addStretch();
}

void RegexTesterGUI::copyPattern()
{
    QApplication::clipboard()->setText(m_patternEdit->text());
}

void RegexTesterGUI::copyMatches()
{
    if (m_lastMatches.isEmpty()) {
        QApplication::clipboard()->setText(tr("No matches"));
        return;
    }

    QString result;
    QTextStream stream(&result);
    stream << tr("Match Results (Count: %1)").arg(m_lastMatches.size()) << "\n\n";

    for (const auto &match : m_lastMatches) {
        QString const matchStr = m_testTextEdit->toPlainText().mid(match.offset, match.length);
        stream << QString("Match %1: \"%2\" [%3, %4]\n")
                      .arg(match.index + 1)
                      .arg(matchStr)
                      .arg(match.offset)
                      .arg(match.offset + match.length);
        for (int i = 1; i < match.groups.size(); ++i) {
            const auto &group = match.groups[i];
            stream << QString("  Group %1: \"%2\" [%3, %4]\n")
                          .arg(group.index)
                          .arg(group.value)
                          .arg(group.offset)
                          .arg(group.offset + group.length);
        }
        stream << "\n";
    }

    QApplication::clipboard()->setText(result);
}

void RegexTesterGUI::copyToClipboard()
{
    QApplication::clipboard()->setText(m_replaceResultEdit->toPlainText());
}

void RegexTesterGUI::onPresetSelected(int index)
{
    if (index == 0)
        return;

    switch (index) {
    case 1: // Email
        m_patternEdit->setText(R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}\b)");
        m_testTextEdit->setPlainText(
            tr("Please contact support@example.com or sales-info@example.org for help."));
        break;
    case 2: // URL
        m_patternEdit->setText(
            R"(https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*))");
        m_testTextEdit->setPlainText(
            tr("Visit https://google.com or http://localhost:8080/index.html."));
        break;
    case 3: // IPv4
        m_patternEdit->setText(
            R"(\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b)");
        m_testTextEdit->setPlainText(
            tr("The local router is at 192.168.1.1 and the DNS is 8.8.8.8."));
        break;
    case 4: // ISO Date
        m_patternEdit->setText(R"(\b\d{4}-\d{2}-\d{2}\b)");
        m_testTextEdit->setPlainText(
            tr("The project started on 2026-07-14 and ends on 2026-12-31."));
        break;
    case 5: // Phone
        m_patternEdit->setText(R"(\+[1-9]\d{1,14})");
        m_testTextEdit->setPlainText(
            tr("Call us at +14155552671 or international line +81312345678."));
        break;
    case 6: // UUID v4
        m_patternEdit->setText(
            R"(\b[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}\b)");
        m_testTextEdit->setPlainText(tr("Generated ID: f47ac10b-58cc-4372-a567-0e02b2c3d479."));
        break;
    }

    m_presetCombo->setCurrentIndex(0);
}

void RegexTesterGUI::toggleQuickRef()
{
    bool const visible = m_quickRefContainer->isVisible();
    m_quickRefContainer->setVisible(!visible);
    m_quickRefToggle->setText(!visible ? tr("Hide Quick Reference") : tr("Show Quick Reference"));
}

void RegexTesterGUI::onQuickRefDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (item->childCount() > 0)
        return; // ignore category headers
    QString const token = item->text(0);
    m_patternEdit->insert(token);
    m_patternEdit->setFocus();
}

} // namespace devtools
