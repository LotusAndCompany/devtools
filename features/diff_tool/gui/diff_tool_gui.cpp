#include "diff_tool_gui.h"

#include "features/diff_tool/core/diff_tool.h"

#include <QComboBox>
#include <QEvent>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QStyleHints>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr int RENDER_DEBOUNCE_MS = 150;
constexpr int DEFAULT_WIDTH = 900;
constexpr int DEFAULT_HEIGHT = 600;
} // namespace

DiffToolGUI::DiffToolGUI(DiffTool *tool, QWidget *parent) : GuiTool(parent), tool(tool)
{
    buildUi();

    // NOTE: parent が設定されていなければこのインスタンスで管理する
    if (tool != nullptr && tool->parent() == nullptr) {
        tool->setParent(this);
    }

    connect(leftEditor, &QPlainTextEdit::textChanged, this, &DiffToolGUI::onTextChanged);
    connect(rightEditor, &QPlainTextEdit::textChanged, this, &DiffToolGUI::onTextChanged);
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DiffToolGUI::onModeChanged);
    connect(renderTimer, &QTimer::timeout, this, &DiffToolGUI::onRenderTimeout);

    renderDiff();
}

void DiffToolGUI::buildUi()
{
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // 表示モード切り替え
    auto *headerRow = new QHBoxLayout();
    headerRow->setSpacing(8);

    modeCombo = new QComboBox(this);
    modeCombo->addItem(QString()); // Inline
    modeCombo->addItem(QString()); // Side-by-side
    headerRow->addWidget(modeCombo);
    headerRow->addStretch();

    mainLayout->addLayout(headerRow);

    // 入力エディタ（左 = 変更前 / 右 = 変更後）
    const QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    auto *splitter = new QSplitter(Qt::Horizontal, this);

    leftEditor = new QPlainTextEdit(splitter);
    leftEditor->setFont(monoFont);
    leftEditor->setLineWrapMode(QPlainTextEdit::NoWrap);

    rightEditor = new QPlainTextEdit(splitter);
    rightEditor->setFont(monoFont);
    rightEditor->setLineWrapMode(QPlainTextEdit::NoWrap);

    auto *leftPane = buildEditorPane(leftLabel, leftEditor);
    auto *rightPane = buildEditorPane(rightLabel, rightEditor);

    splitter->addWidget(leftPane);
    splitter->addWidget(rightPane);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter, 1);

    // 差分表示
    resultView = new QTextBrowser(this);
    resultView->setOpenExternalLinks(false);
    resultView->setFont(monoFont);

    mainLayout->addWidget(resultView, 1);

    renderTimer = new QTimer(this);
    renderTimer->setSingleShot(true);
    renderTimer->setInterval(RENDER_DEBOUNCE_MS);

    retranslateUi();
}

QWidget *DiffToolGUI::buildEditorPane(QLabel *&caption, QPlainTextEdit *editor)
{
    auto *pane = new QWidget(this);

    auto *layout = new QVBoxLayout(pane);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    caption = new QLabel(pane);
    layout->addWidget(caption);
    layout->addWidget(editor);

    return pane;
}

void DiffToolGUI::retranslateUi()
{
    setWindowTitle(tr("Diff Comparison Tool"));
    modeCombo->setItemText(0, tr("Inline"));
    modeCombo->setItemText(1, tr("Side-by-side"));
    leftLabel->setText(tr("Original"));
    rightLabel->setText(tr("Modified"));
    leftEditor->setPlaceholderText(tr("Paste original text here..."));
    rightEditor->setPlaceholderText(tr("Paste modified text here..."));
}

void DiffToolGUI::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        event->accept();
    } else if (event->type() == QEvent::PaletteChange) {
        // ダークモード切り替えで配色が変わるため差分表示を再描画する
        renderDiff();
        event->accept();
    } else {
        QWidget::changeEvent(event);
    }
}

void DiffToolGUI::onTextChanged()
{
    // 入力のたびにタイマーを再スタートしてデバウンスする
    renderTimer->start();
}

void DiffToolGUI::onModeChanged(int index)
{
    Q_UNUSED(index);
    renderDiff();
}

void DiffToolGUI::onRenderTimeout()
{
    renderDiff();
}

void DiffToolGUI::renderDiff()
{
    const auto lines = DiffTool::diff(leftEditor->toPlainText(), rightEditor->toPlainText());

    if (modeCombo->currentIndex() == 0) {
        resultView->setHtml(buildInlineHtml(lines));
    } else {
        resultView->setHtml(buildSideBySideHtml(lines));
    }
}

QString DiffToolGUI::buildStyleSheet()
{
    const bool dark = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
    const QString textColor = QGuiApplication::palette().color(QPalette::Text).name();
    const QString addBg = dark ? QStringLiteral("#143a28") : QStringLiteral("#e6ffec");
    const QString delBg = dark ? QStringLiteral("#3a2020") : QStringLiteral("#ffebe9");

    return QStringLiteral(
               "<style>"
               "body{font-family:monospace;color:%1;}"
               ".add{background-color:%2;}"
               ".del{background-color:%3;}"
               "table{border-collapse:collapse;}"
               "td{padding:0 8px;vertical-align:top;}"
               "</style>")
        .arg(textColor, addBg, delBg);
}

QString DiffToolGUI::buildInlineHtml(const QVector<DiffLine> &lines)
{
    QString html = QStringLiteral("<html><head>");
    html += buildStyleSheet();
    html += QStringLiteral("</head><body>");

    for (const auto &line : lines) {
        QString text = line.text.toHtmlEscaped();
        if (text.isEmpty()) {
            text = QStringLiteral("&nbsp;");
        }

        if (line.operation == DiffOperation::Equal) {
            html += QStringLiteral("<div>%1</div>").arg(text);
        } else if (line.operation == DiffOperation::Delete) {
            html += QStringLiteral("<div class=\"del\">- %1</div>").arg(text);
        } else {
            html += QStringLiteral("<div class=\"add\">+ %1</div>").arg(text);
        }
    }

    html += QStringLiteral("</body></html>");
    return html;
}

QString DiffToolGUI::buildSideBySideHtml(const QVector<DiffLine> &lines)
{
    const auto rows = DiffTool::groupSideBySide(lines);

    QString html = QStringLiteral("<html><head>");
    html += buildStyleSheet();
    html += QStringLiteral("</head><body><table>");

    for (const auto &row : rows) {
        const QString leftText =
            row.left_text.isEmpty() ? QStringLiteral("&nbsp;") : row.left_text.toHtmlEscaped();
        const QString rightText =
            row.right_text.isEmpty() ? QStringLiteral("&nbsp;") : row.right_text.toHtmlEscaped();
        const QString leftClass = row.left_changed ? QStringLiteral("del") : QString();
        const QString rightClass = row.right_changed ? QStringLiteral("add") : QString();

        html += QStringLiteral("<tr><td class=\"%1\">%2</td><td class=\"%3\">%4</td></tr>")
                    .arg(leftClass, leftText, rightClass, rightText);
    }

    html += QStringLiteral("</table></body></html>");
    return html;
}
