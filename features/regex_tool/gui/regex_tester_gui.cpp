#include "features/regex_tool/gui/regex_tester_gui.h"

#include "features/framework/gui/design_system.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QEvent>
#include <QFrame>
#include <QGroupBox>
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

namespace {

/**
 * @brief 正規表現のマッチを収集する（中断要求を随時確認する）
 */
QVector<MatchResult> collectMatches(const QRegularExpression &re, const QString &text, bool global)
{
    QVector<MatchResult> matches;
    const QStringList capture_group_names = re.namedCaptureGroups();

    if (global) {
        QRegularExpressionMatchIterator i = re.globalMatch(text);
        int match_index = 0;
        while (i.hasNext()) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                return matches;
            }
            const QRegularExpressionMatch match = i.next();
            MatchResult res;
            res.index = match_index++;
            res.offset = match.capturedStart();
            res.length = match.capturedLength();

            for (int j = 0; j <= match.lastCapturedIndex(); ++j) {
                CaptureGroup group;
                group.index = j;
                group.name = capture_group_names.value(j);
                group.value = match.captured(j);
                group.offset = match.capturedStart(j);
                group.length = match.capturedLength(j);
                res.groups.append(group);
            }
            matches.append(res);
        }
    } else {
        const QRegularExpressionMatch match = re.match(text);
        if (match.hasMatch()) {
            MatchResult res;
            res.index = 0;
            res.offset = match.capturedStart();
            res.length = match.capturedLength();

            for (int j = 0; j <= match.lastCapturedIndex(); ++j) {
                CaptureGroup group;
                group.index = j;
                group.name = capture_group_names.value(j);
                group.value = match.captured(j);
                group.offset = match.capturedStart(j);
                group.length = match.capturedLength(j);
                res.groups.append(group);
            }
            matches.append(res);
        }
    }

    return matches;
}

} // namespace

// ============================================================================
// RegexWorker Implementation
// ============================================================================

RegexWorker::RegexWorker(QObject *parent)
    : QThread(parent)
    , m_requestId(0)
    , m_options(QRegularExpression::NoPatternOption)
    , m_global(true)
{}

void RegexWorker::setParams(int requestId, const QString &pattern, const QString &text,
                            const QString &replacePattern,
                            QRegularExpression::PatternOptions options, bool global)
{
    const QMutexLocker locker(&m_mutex);
    m_requestId = requestId;
    m_pattern = pattern;
    m_text = text;
    m_replacePattern = replacePattern;
    m_options = options;
    m_global = global;
}

void RegexWorker::run()
{
    int requestId;
    QString pattern;
    QString text;
    QString replacePattern;
    QRegularExpression::PatternOptions options;
    bool global;
    {
        const QMutexLocker locker(&m_mutex);
        requestId = m_requestId;
        pattern = m_pattern;
        text = m_text;
        replacePattern = m_replacePattern;
        options = m_options;
        global = m_global;
    }

    if (pattern.isEmpty()) {
        emit finishedMatching(requestId, QVector<MatchResult>(), text, true, QString());
        return;
    }

    const QRegularExpression re(pattern, options);
    if (!re.isValid()) {
        emit finishedMatching(requestId, QVector<MatchResult>(), text, false, re.errorString());
        return;
    }

    const QVector<MatchResult> matches = collectMatches(re, text, global);

    if (isInterruptionRequested()) {
        return;
    }

    // Execute replace using custom ECMA-like substitution
    const QString replacedText = RegexTool::replace(pattern, text, replacePattern, options, global);

    emit finishedMatching(requestId, matches, replacedText, true, QString());
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
    const qsizetype blockStart = currentBlock().position();
    const qsizetype blockLength = text.length();

    for (const auto &match : m_matches) {
        const qsizetype start = match.offset;
        const qsizetype end = match.offset + match.length;

        if (end <= blockStart || start >= blockStart + blockLength) {
            continue;
        }

        const int relativeStart = static_cast<int>(qMax<qsizetype>(0, start - blockStart));
        const int relativeLength =
            static_cast<int>(qMin(blockLength - relativeStart, end - (blockStart + relativeStart)));

        setFormat(relativeStart, relativeLength, m_matchFormat);
    }
}

// ============================================================================
// RegexTesterGUI Implementation
// ============================================================================

RegexTesterGUI::RegexTesterGUI(QWidget *parent) : GuiTool(parent)
{
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
    if (m_worker != nullptr) {
        m_worker->disconnect(this);
        m_worker->requestInterruption();
        m_worker->wait();
        delete m_worker;
        m_worker = nullptr;
    }
}

void RegexTesterGUI::setupUi()
{
    auto *const mainLayout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(mainLayout);

    auto *const splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(splitter);

    splitter->addWidget(setupLeftPane(splitter));
    splitter->addWidget(setupRightPane(splitter));
    DevTools::Ui::configureMainSideSplitter(splitter);
}

QWidget *RegexTesterGUI::setupLeftPane(QWidget *parent)
{
    auto *const leftWidget = DevTools::Ui::createPane(tr("Regex Tester"), parent);
    m_leftPane = leftWidget;
    auto *const leftLayout = new QVBoxLayout(leftWidget);
    DevTools::Ui::applyPanelLayout(leftLayout);

    // Presets Row
    auto *const presetsRow = new QHBoxLayout();
    DevTools::Ui::applyInlineLayout(presetsRow);
    auto *const presetCombo = createPresetCombo(leftWidget);
    DevTools::Ui::configureComboBox(presetCombo);
    presetsRow->addWidget(presetCombo, 1);

    m_copyPatternButton = new QPushButton(tr("Copy Pattern"), leftWidget);
    DevTools::Ui::configureCompactButton(m_copyPatternButton);
    presetsRow->addWidget(m_copyPatternButton);
    leftLayout->addLayout(presetsRow);

    // Pattern Edit
    m_patternEdit = new QLineEdit(leftWidget);
    m_patternEdit->setPlaceholderText(tr("Regular Expression Pattern"));
    DevTools::Ui::configureCodeLineEdit(m_patternEdit);
    leftLayout->addWidget(m_patternEdit);

    // Flags Row
    auto *const flagsLayout = new QHBoxLayout();
    DevTools::Ui::applyInlineLayout(flagsLayout);
    setupFlagButtons(leftWidget, flagsLayout);
    flagsLayout->addStretch();
    leftLayout->addLayout(flagsLayout);

    // Error Label
    m_errorLabel = DevTools::Ui::createErrorLabel(leftWidget);
    DevTools::Ui::configureCodeLabel(m_errorLabel);
    m_errorLabel->setWordWrap(true);
    leftLayout->addWidget(m_errorLabel);

    // Test Text Label
    m_testTextHeading = DevTools::Ui::createPaneHeading(tr("Test Text"), leftWidget);
    leftLayout->addWidget(m_testTextHeading);

    // Test Text Edit
    m_testTextEdit = new QPlainTextEdit(leftWidget);
    m_testTextEdit->setPlaceholderText(tr("Enter text to test against..."));
    DevTools::Ui::configureCodeEditor(m_testTextEdit);
    leftLayout->addWidget(m_testTextEdit, 2);

    // Replace Label / Row / Result
    setupReplaceSection(leftWidget, leftLayout);

    return leftWidget;
}

QComboBox *RegexTesterGUI::createPresetCombo(QWidget *parent)
{
    m_presetCombo = new QComboBox(parent);
    m_presetCombo->addItem(tr("Select preset pattern..."));
    m_presetCombo->addItem(tr("Email Address"));
    m_presetCombo->addItem(tr("URL (Web Address)"));
    m_presetCombo->addItem(tr("IPv4 Address"));
    m_presetCombo->addItem(tr("ISO Date (YYYY-MM-DD)"));
    m_presetCombo->addItem(tr("Phone Number (E.164)"));
    m_presetCombo->addItem(tr("UUID v4"));
    return m_presetCombo;
}

void RegexTesterGUI::setupReplaceSection(QWidget *parent, QVBoxLayout *layout)
{
    m_replaceHeading = DevTools::Ui::createPaneHeading(tr("Substitution / Replace"), parent);
    layout->addWidget(m_replaceHeading);

    auto *const replaceRow = new QHBoxLayout();
    DevTools::Ui::applyInlineLayout(replaceRow);
    m_replacePatternEdit = new QLineEdit(parent);
    m_replacePatternEdit->setPlaceholderText(tr("Replacement String"));
    DevTools::Ui::configureCodeLineEdit(m_replacePatternEdit);
    replaceRow->addWidget(m_replacePatternEdit, 1);

    m_copyResultButton = new QPushButton(tr("Copy Result"), parent);
    DevTools::Ui::configureCompactButton(m_copyResultButton);
    replaceRow->addWidget(m_copyResultButton);
    layout->addLayout(replaceRow);

    m_replaceResultEdit = new QPlainTextEdit(parent);
    m_replaceResultEdit->setReadOnly(true);
    m_replaceResultEdit->setPlaceholderText(tr("Replacement result will appear here..."));
    DevTools::Ui::configureDisplayTextControl(m_replaceResultEdit);
    layout->addWidget(m_replaceResultEdit, 1);
}

void RegexTesterGUI::setupFlagButtons(QWidget *parent, QHBoxLayout *layout)
{
    m_flagG = new QToolButton(parent);
    m_flagG->setText("g");
    m_flagG->setCheckable(true);
    m_flagG->setChecked(true);
    m_flagG->setToolTip(tr("Global (match all)"));
    DevTools::Ui::configureCompactButton(m_flagG);
    layout->addWidget(m_flagG);

    m_flagI = new QToolButton(parent);
    m_flagI->setText("i");
    m_flagI->setCheckable(true);
    m_flagI->setToolTip(tr("Case Insensitive"));
    DevTools::Ui::configureCompactButton(m_flagI);
    layout->addWidget(m_flagI);

    m_flagM = new QToolButton(parent);
    m_flagM->setText("m");
    m_flagM->setCheckable(true);
    m_flagM->setToolTip(tr("Multiline"));
    DevTools::Ui::configureCompactButton(m_flagM);
    layout->addWidget(m_flagM);

    m_flagS = new QToolButton(parent);
    m_flagS->setText("s");
    m_flagS->setCheckable(true);
    m_flagS->setToolTip(tr("Dot matches all (Singleline)"));
    DevTools::Ui::configureCompactButton(m_flagS);
    layout->addWidget(m_flagS);

    m_flagU = new QToolButton(parent);
    m_flagU->setText("u");
    m_flagU->setCheckable(true);
    m_flagU->setToolTip(tr("Unicode"));
    DevTools::Ui::configureCompactButton(m_flagU);
    layout->addWidget(m_flagU);

    m_flagX = new QToolButton(parent);
    m_flagX->setText("x");
    m_flagX->setCheckable(true);
    m_flagX->setToolTip(tr("Extended (ignore whitespace in pattern)"));
    DevTools::Ui::configureCompactButton(m_flagX);
    layout->addWidget(m_flagX);
}

QWidget *RegexTesterGUI::setupRightPane(QWidget *parent)
{
    auto *const rightWidget = DevTools::Ui::createPane(tr("Matches"), parent);
    m_rightPane = rightWidget;
    auto *const rightLayout = new QVBoxLayout(rightWidget);
    DevTools::Ui::applyPanelLayout(rightLayout);

    // Matches Header Row
    auto *const matchesHeaderRow = new QHBoxLayout();
    DevTools::Ui::applyInlineLayout(matchesHeaderRow);
    m_matchCountLabel = DevTools::Ui::createPaneHeading(tr("Matches (0)"), rightWidget);
    matchesHeaderRow->addWidget(m_matchCountLabel);

    matchesHeaderRow->addStretch();

    m_copyMatchesButton = new QPushButton(tr("Copy Matches"), rightWidget);
    DevTools::Ui::configureCompactButton(m_copyMatchesButton);
    matchesHeaderRow->addWidget(m_copyMatchesButton);
    rightLayout->addLayout(matchesHeaderRow);

    // Match Result Scroll Area
    auto *const matchResultArea = new QScrollArea(rightWidget);
    DevTools::Ui::configureScrollView(matchResultArea);
    auto *const matchResultContent = new QWidget();
    DevTools::Ui::configurePaneSurface(matchResultContent);
    m_matchResultLayout = new QVBoxLayout(matchResultContent);
    DevTools::Ui::applyPanelLayout(m_matchResultLayout);
    matchResultArea->setWidget(matchResultContent);
    rightLayout->addWidget(matchResultArea, 2);

    // Quick Reference Collapsible Section
    auto *const quickReferencePane = DevTools::Ui::createPane(tr("Quick Reference"), rightWidget);
    m_quickReferencePane = quickReferencePane;
    auto *const quickReferenceLayout = new QVBoxLayout(quickReferencePane);
    DevTools::Ui::applyPanelLayout(quickReferenceLayout);

    m_quickRefToggle = new QPushButton(tr("Hide Quick Reference"), quickReferencePane);
    DevTools::Ui::configureCompactButton(m_quickRefToggle);
    quickReferenceLayout->addWidget(m_quickRefToggle, 0, Qt::AlignLeft);

    m_quickRefContainer = new QWidget(quickReferencePane);
    auto *const quickRefLayout = new QVBoxLayout(m_quickRefContainer);
    DevTools::Ui::applyInlineLayout(quickRefLayout);

    m_quickRefTree = new QTreeWidget(m_quickRefContainer);
    DevTools::Ui::configureItemView(m_quickRefTree);
    m_quickRefTree->setHeaderHidden(true);
    m_quickRefTree->setColumnCount(2);
    m_quickRefTree->setColumnWidth(0, 110);
    m_quickRefTree->setToolTip(tr("Double-click to insert token into pattern"));

    populateQuickReference(m_quickRefTree);

    m_quickRefTree->expandAll();
    quickRefLayout->addWidget(m_quickRefTree);
    quickReferenceLayout->addWidget(m_quickRefContainer, 1);
    rightLayout->addWidget(quickReferencePane, 1);

    return rightWidget;
}

void RegexTesterGUI::populateQuickReference(QTreeWidget *tree)
{
    // Character Classes
    auto *charClasses = new QTreeWidgetItem(tree, QStringList() << tr("Character Classes"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\d)" << tr("Digit (0-9)"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\D)" << tr("Non-digit"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\w)" << tr("Word char (a-z, 0-9, _)"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\W)" << tr("Non-word char"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\s)" << tr("Whitespace"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(\S)" << tr("Non-whitespace"));
    new QTreeWidgetItem(charClasses, QStringList() << R"(.)" << tr("Any character except newline"));

    // Quantifiers
    auto *quantifiers = new QTreeWidgetItem(tree, QStringList() << tr("Quantifiers"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"(*)" << tr("0 or more times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"(+)" << tr("1 or more times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"(?)" << tr("0 or 1 time (optional)"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"({n})" << tr("Exactly n times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"({n,})" << tr("n or more times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"({n,m})" << tr("Between n and m times"));
    new QTreeWidgetItem(quantifiers, QStringList() << R"(*?)" << tr("Lazy quantifier"));

    // Anchors
    auto *anchors = new QTreeWidgetItem(tree, QStringList() << tr("Anchors"));
    new QTreeWidgetItem(anchors, QStringList() << R"(^)" << tr("Start of string / line"));
    new QTreeWidgetItem(anchors, QStringList() << R"($)" << tr("End of string / line"));
    new QTreeWidgetItem(anchors, QStringList() << R"(\b)" << tr("Word boundary"));
    new QTreeWidgetItem(anchors, QStringList() << R"(\B)" << tr("Non-word boundary"));

    // Groups & References
    auto *groups = new QTreeWidgetItem(tree, QStringList() << tr("Groups & References"));
    new QTreeWidgetItem(groups, QStringList() << R"((...))" << tr("Capture group"));
    new QTreeWidgetItem(groups, QStringList() << R"((?:...))" << tr("Non-capturing group"));
    new QTreeWidgetItem(groups, QStringList() << R"((?<name>...))" << tr("Named capture group"));
    new QTreeWidgetItem(groups, QStringList() << R"(\1)" << tr("Match group #1 reference"));
    new QTreeWidgetItem(groups, QStringList()
                                    << R"(\k<name>)" << tr("Match named group reference"));
}

void RegexTesterGUI::retranslateUi()
{
    const bool quickReferenceVisible = m_quickRefContainer->isVisible();

    m_leftPane->setTitle(tr("Regex Tester"));
    m_rightPane->setTitle(tr("Matches"));
    m_quickReferencePane->setTitle(tr("Quick Reference"));
    m_testTextHeading->setText(tr("Test Text"));
    m_replaceHeading->setText(tr("Substitution / Replace"));

    m_presetCombo->setItemText(0, tr("Select preset pattern..."));
    m_presetCombo->setItemText(1, tr("Email Address"));
    m_presetCombo->setItemText(2, tr("URL (Web Address)"));
    m_presetCombo->setItemText(3, tr("IPv4 Address"));
    m_presetCombo->setItemText(4, tr("ISO Date (YYYY-MM-DD)"));
    m_presetCombo->setItemText(5, tr("Phone Number (E.164)"));
    m_presetCombo->setItemText(6, tr("UUID v4"));

    m_copyPatternButton->setText(tr("Copy Pattern"));
    m_patternEdit->setPlaceholderText(tr("Regular Expression Pattern"));
    m_flagG->setToolTip(tr("Global (match all)"));
    m_flagI->setToolTip(tr("Case Insensitive"));
    m_flagM->setToolTip(tr("Multiline"));
    m_flagS->setToolTip(tr("Dot matches all (Singleline)"));
    m_flagU->setToolTip(tr("Unicode"));
    m_flagX->setToolTip(tr("Extended (ignore whitespace in pattern)"));
    m_testTextEdit->setPlaceholderText(tr("Enter text to test against..."));
    m_replacePatternEdit->setPlaceholderText(tr("Replacement String"));
    m_copyResultButton->setText(tr("Copy Result"));
    m_replaceResultEdit->setPlaceholderText(tr("Replacement result will appear here..."));
    m_copyMatchesButton->setText(tr("Copy Matches"));
    m_quickRefToggle->setText(quickReferenceVisible ? tr("Hide Quick Reference")
                                                    : tr("Show Quick Reference"));
    m_quickRefTree->setToolTip(tr("Double-click to insert token into pattern"));

    m_quickRefTree->clear();
    populateQuickReference(m_quickRefTree);
    m_quickRefTree->expandAll();
    m_quickRefContainer->setVisible(quickReferenceVisible);

    updateMatchResultDisplay(m_lastMatches);
    m_errorLabel->clear();
    m_errorLabel->hide();
    triggerUpdate();
}

void RegexTesterGUI::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        event->accept();
    } else {
        QWidget::changeEvent(event);
    }
}

void RegexTesterGUI::setupConnections()
{
    connect(m_patternEdit, &QLineEdit::textChanged, this, &RegexTesterGUI::triggerUpdate);
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::activated), this,
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
}

void RegexTesterGUI::loadSettings()
{
    const QSettings settings;
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

    // Abandon the previous worker without blocking the GUI thread. It stops
    // cooperatively at its next interruption check and is deleted via the
    // finished signal once it completes.
    if (m_worker != nullptr) {
        m_worker->requestInterruption();
        m_worker = nullptr;
    }

    const QString pattern = m_patternEdit->text();
    const QString text = m_testTextEdit->toPlainText();
    const QString replacePattern = m_replacePatternEdit->text();
    const auto options = currentOptions();
    bool const global = m_flagG->isChecked();
    const int requestId = ++m_requestId;

    auto *const worker = new RegexWorker;
    m_worker = worker;
    connect(worker, &RegexWorker::finishedMatching, this, &RegexTesterGUI::onMatchingFinished);
    connect(worker, &QThread::finished, this, [this, worker] {
        if (m_worker == worker) {
            m_worker = nullptr;
        }
        worker->deleteLater();
    });
    worker->setParams(requestId, pattern, text, replacePattern, options, global);
    worker->start();
    m_watchdogTimer->start(500); // 500ms limit
}

void RegexTesterGUI::onMatchingFinished(int requestId, const QVector<MatchResult> &matches,
                                        const QString &replacedText, bool isValid,
                                        const QString &errorStr)
{
    // Discard stale notifications from abandoned workers.
    if (requestId != m_requestId) {
        return;
    }

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
    m_watchdogTimer->stop();

    // Invalidate queued results even if the worker stopped before this slot ran.
    ++m_requestId;
    if (m_worker != nullptr && m_worker->isRunning()) {
        m_worker->requestInterruption();
    }

    m_errorLabel->setText(tr("Error: Evaluation timed out (catastrophic backtracking detected)"));
    m_errorLabel->show();

    m_lastMatches.clear();
    m_highlighter->setMatches(m_lastMatches);
    updateMatchResultDisplay(m_lastMatches);
    m_replaceResultEdit->setPlainText(m_testTextEdit->toPlainText());
}

void RegexTesterGUI::updateMatchResultDisplay(const QVector<MatchResult> &matches)
{
    // Clear old result
    QLayoutItem *item;
    while ((item = m_matchResultLayout->takeAt(0)) != nullptr) {
        if (item->widget() != nullptr) {
            delete item->widget();
        }
        delete item;
    }

    m_matchCountLabel->setText(tr("Matches (%1)").arg(matches.size()));

    if (matches.isEmpty()) {
        auto *const label = new QLabel(tr("No matches"), this);
        DevTools::Ui::configureCodeLabel(label);
        m_matchResultLayout->addWidget(label);
    } else {
        for (const auto &match : matches) {
            QString const matchText = m_testTextEdit->toPlainText().mid(match.offset, match.length);
            QString const resultText = tr("Match %1: [%2, %3]\n\"%4\"")
                                           .arg(match.index + 1)
                                           .arg(match.offset)
                                           .arg(match.offset + match.length)
                                           .arg(matchText);

            auto *const label = new QLabel(resultText, this);
            label->setWordWrap(true);
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            DevTools::Ui::configureCodeLabel(label);
            m_matchResultLayout->addWidget(label);

            for (int i = 1; i < match.groups.size(); ++i) {
                const auto &group = match.groups.at(i);
                const QString groupIdentifier =
                    group.name.isEmpty() ? QString::number(group.index)
                                         : QString("%1 (%2)").arg(group.index).arg(group.name);
                QString const groupText = tr("  Group %1: [%2, %3]\n  \"%4\"")
                                              .arg(groupIdentifier)
                                              .arg(group.offset)
                                              .arg(group.offset + group.length)
                                              .arg(group.value);
                auto *const groupLabel = new QLabel(groupText, this);
                groupLabel->setWordWrap(true);
                groupLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
                DevTools::Ui::configureCodeLabel(groupLabel);
                groupLabel->setIndent(DevTools::Ui::Metrics::PANEL_MARGIN);
                m_matchResultLayout->addWidget(groupLabel);
            }

            auto *const divider = new QFrame(this);
            DevTools::Ui::configureDivider(divider);
            m_matchResultLayout->addWidget(divider);
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
        stream << tr("Match %1: \"%2\" [%3, %4]\n")
                      .arg(match.index + 1)
                      .arg(matchStr)
                      .arg(match.offset)
                      .arg(match.offset + match.length);
        for (int i = 1; i < match.groups.size(); ++i) {
            const auto &group = match.groups.at(i);
            const QString groupIdentifier =
                group.name.isEmpty() ? QString::number(group.index)
                                     : QString("%1 (%2)").arg(group.index).arg(group.name);
            stream << tr("  Group %1: \"%2\" [%3, %4]\n")
                          .arg(groupIdentifier)
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
    if (index == 0) {
        return;
    }

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
    default:
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
    if (item->childCount() > 0) {
        return; // ignore category headers
    }
    QString const token = item->text(0);
    m_patternEdit->insert(token);
    m_patternEdit->setFocus();
}

} // namespace devtools
