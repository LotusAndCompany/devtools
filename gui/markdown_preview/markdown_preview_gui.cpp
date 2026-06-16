#include "markdown_preview_gui.h"

#include "core/markdown_preview/markdown_preview.h"

#include <QAbstractSlider>
#include <QAction>
#include <QCheckBox>
#include <QColor>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QPalette>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QSizePolicy>
#include <QSplitter>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include <cmath>

namespace {
constexpr int RENDER_DEBOUNCE_MS = 180;
constexpr int DEFAULT_WIDTH = 800;
constexpr int DEFAULT_HEIGHT = 600;

// 2 色を t (0.0-1.0) の割合で線形補間する。テーマのパレットから
// ボーダー / ミュート文字 / ホバー色などの中間色を導出するために使う。
QColor blend(const QColor &a, const QColor &b, double t)
{
    return QColor::fromRgbF(a.redF() * (1.0 - t) + b.redF() * t,
                            a.greenF() * (1.0 - t) + b.greenF() * t,
                            a.blueF() * (1.0 - t) + b.blueF() * t);
}

// .pen の見た目 (カード状ペイン / 角丸ボタン) を、ハードコードした配色ではなく
// 現在のテーマのパレットから導出して再現する。これによりライト / ダークの
// どちらでもアプリ本体と調和する。
QString buildStyleSheet(const QPalette &pal)
{
    const QColor window = pal.color(QPalette::Window);
    const QColor text = pal.color(QPalette::Text);

    // ウィンドウ背景からわずかに浮かせたカード面を作る (ライト/ダーク共通)
    const QColor card = blend(window, text, 0.05);
    const QColor border = blend(window, text, 0.16);
    const QColor muted = blend(window, text, 0.45);
    const QColor hover = blend(card, text, 0.08);
    const QColor pressed = blend(card, text, 0.16);

    return QString(R"(
MarkdownPreviewGUI { background-color: %1; }
QToolBar { background: transparent; border: none; spacing: 6px; }
QToolBar::separator { background: %3; width: 1px; margin: 4px 6px; }
QToolButton {
    background-color: %2;
    color: %4;
    border: 1px solid %3;
    border-radius: 8px;
    padding: 6px 12px;
}
QToolButton:hover { background-color: %5; }
QToolButton:pressed { background-color: %6; }
QWidget#editorPane, QWidget#previewPane {
    background-color: %2;
    border-radius: 16px;
}
#editorPane QLabel, #previewPane QLabel {
    color: %7;
    font-size: 11px;
    font-weight: bold;
}
QPlainTextEdit, QTextBrowser {
    background: transparent;
    color: %4;
    border: none;
}
QSplitter::handle { background: transparent; }
)")
        .arg(window.name(), card.name(), border.name(), text.name(), hover.name(), pressed.name(),
             muted.name());
}
} // namespace

MarkdownPreviewGUI::MarkdownPreviewGUI(MarkdownPreview *tool, QWidget *parent)
    : GuiTool(parent), tool(tool)
{
    buildUi();

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (tool != nullptr && tool->parent() == nullptr) {
        tool->setParent(this);
    }

    connect(openAction, &QAction::triggered, this, &MarkdownPreviewGUI::onOpenClicked);
    connect(saveAction, &QAction::triggered, this, &MarkdownPreviewGUI::onSaveClicked);
    connect(exportHtmlAction, &QAction::triggered, this, &MarkdownPreviewGUI::onExportHtmlClicked);
    connect(syncScrollCheck, &QCheckBox::toggled, this, &MarkdownPreviewGUI::onSyncScrollToggled);
    connect(editor, &QPlainTextEdit::textChanged, this, &MarkdownPreviewGUI::onEditorTextChanged);
    connect(renderTimer, &QTimer::timeout, this, &MarkdownPreviewGUI::onRenderTimeout);
    connect(editor->verticalScrollBar(), &QAbstractSlider::valueChanged, this,
            &MarkdownPreviewGUI::onEditorScrolled);
    connect(preview->verticalScrollBar(), &QAbstractSlider::valueChanged, this,
            &MarkdownPreviewGUI::onPreviewScrolled);
}

void MarkdownPreviewGUI::buildUi()
{
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    toolbar = new QToolBar(this);
    // 専用アイコン (file/save 等) はリソース未登録のため、空アイコンで隙間が
    // 空くのを避けてテキストのみのボタンにする。
    toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    openAction = toolbar->addAction(QString());
    saveAction = toolbar->addAction(QString());
    exportHtmlAction = toolbar->addAction(QString());
    toolbar->addSeparator();

    auto *spacer = new QWidget(toolbar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    syncScrollCheck = new QCheckBox(toolbar);
    syncScrollCheck->setChecked(syncScrollEnabled);
    toolbar->addWidget(syncScrollCheck);

    mainLayout->addWidget(toolbar);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(16); // .pen の panes gap (16px) に合わせる

    editor = new QPlainTextEdit;
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    editor->setFrameShape(QFrame::NoFrame);
    // .pen のエディタは等幅フォント。インストール環境に依存しないよう
    // システム標準の等幅フォントを使う。
    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    editor->setFont(monoFont);

    preview = new QTextBrowser;
    preview->setOpenExternalLinks(true);
    preview->setFrameShape(QFrame::NoFrame);
    // レンダリング結果が等幅にならないよう、本文は UI 標準フォントにする。
    preview->setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    // ドキュメント周囲に余白を持たせて、カードの padding と二重に詰まらないよう調整。
    preview->document()->setDocumentMargin(4);

    auto *editorPane = buildPane(editorLabel, editor);
    editorPane->setObjectName("editorPane");
    auto *previewPane = buildPane(previewLabel, preview);
    previewPane->setObjectName("previewPane");

    splitter->addWidget(editorPane);
    splitter->addWidget(previewPane);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter);

    renderTimer = new QTimer(this);
    renderTimer->setSingleShot(true);
    renderTimer->setInterval(RENDER_DEBOUNCE_MS);

    applyTheme();

    retranslateUi();
}

void MarkdownPreviewGUI::applyTheme()
{
    // setStyleSheet() / setPalette() は内部で PaletteChange を再発火し、
    // それを changeEvent() が拾って applyTheme() を再帰呼び出しする。
    // 再入ガードで無限再帰 (スタックオーバーフロー) を防ぐ。
    if (applyingTheme) {
        return;
    }
    applyingTheme = true;

    const QPalette pal = palette();
    setStyleSheet(buildStyleSheet(pal));

    // QTextBrowser のリンク色はスタイルシートではなくパレットの Link ロールで
    // 決まるため、アクセント色を明示的に設定する。
    QPalette previewPalette = preview->palette();
    previewPalette.setColor(QPalette::Link, pal.color(QPalette::Highlight));
    preview->setPalette(previewPalette);

    applyingTheme = false;
}

QWidget *MarkdownPreviewGUI::buildPane(QLabel *&sectionLabel, QWidget *content)
{
    auto *pane = new QWidget(this);
    // QSS の background-color / border-radius をプレーンな QWidget に描画させる
    pane->setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QVBoxLayout(pane);
    // .pen のカード padding [16, 20] (上下16 / 左右20) に合わせる
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(8);

    sectionLabel = new QLabel(pane);

    layout->addWidget(sectionLabel);
    layout->addWidget(content);

    return pane;
}

void MarkdownPreviewGUI::retranslateUi()
{
    setWindowTitle(tr("Markdown Preview"));
    openAction->setText(tr("Open"));
    saveAction->setText(tr("Save"));
    exportHtmlAction->setText(tr("Export HTML"));
    syncScrollCheck->setText(tr("Sync scroll"));
    editorLabel->setText(tr("Editor"));
    previewLabel->setText(tr("Preview"));
    editor->setPlaceholderText(tr("Type Markdown here..."));
}

void MarkdownPreviewGUI::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        event->accept();
        break;
    case QEvent::PaletteChange:
    case QEvent::ApplicationPaletteChange:
        // ライト / ダーク切り替えに追従してスタイルを再構築する。
        applyTheme();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void MarkdownPreviewGUI::onOpenClicked()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open Markdown File"), currentFilePath, tr("Markdown Files (*.md *.markdown)"));
    if (path.isEmpty()) {
        return;
    }

    const std::optional<QString> content = MarkdownPreview::readFile(path);
    if (!content.has_value()) {
        QMessageBox::warning(this, tr("Markdown Preview"), tr("Failed to open file"));
        return;
    }

    editor->setPlainText(content.value());
    currentFilePath = path;
    renderPreview();
}

void MarkdownPreviewGUI::onSaveClicked()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save Markdown File"), currentFilePath,
                                                tr("Markdown Files (*.md *.markdown)"));
    if (path.isEmpty()) {
        return;
    }

    if (QFileInfo(path).suffix().isEmpty()) {
        path += ".md";
    }

    if (!MarkdownPreview::writeFile(path, editor->toPlainText())) {
        QMessageBox::warning(this, tr("Markdown Preview"), tr("Failed to save file"));
        return;
    }

    currentFilePath = path;
}

void MarkdownPreviewGUI::onExportHtmlClicked()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Export HTML"), QString(),
                                                tr("HTML Files (*.html *.htm)"));
    if (path.isEmpty()) {
        return;
    }

    if (QFileInfo(path).suffix().isEmpty()) {
        path += ".html";
    }

    if (!MarkdownPreview::exportHtml(path, editor->toPlainText())) {
        QMessageBox::warning(this, tr("Markdown Preview"), tr("Failed to export HTML"));
    }
}

void MarkdownPreviewGUI::onSyncScrollToggled(bool enabled)
{
    syncScrollEnabled = enabled;
}

void MarkdownPreviewGUI::onEditorTextChanged()
{
    // 入力のたびにタイマーを再スタートしてデバウンスする
    renderTimer->start();
}

void MarkdownPreviewGUI::onRenderTimeout()
{
    renderPreview();
}

void MarkdownPreviewGUI::renderPreview()
{
    preview->document()->setMarkdown(editor->toPlainText(), QTextDocument::MarkdownDialectGitHub);
}

void MarkdownPreviewGUI::onEditorScrolled()
{
    syncScroll(editor->verticalScrollBar(), preview->verticalScrollBar());
}

void MarkdownPreviewGUI::onPreviewScrolled()
{
    syncScroll(preview->verticalScrollBar(), editor->verticalScrollBar());
}

void MarkdownPreviewGUI::syncScroll(QAbstractSlider *source, QAbstractSlider *target)
{
    if (!syncScrollEnabled || suppressScrollSignal) {
        return;
    }

    const int sourceMax = source->maximum();
    if (sourceMax <= 0) {
        return;
    }

    const double ratio = static_cast<double>(source->value()) / sourceMax;

    // 双方向同期が往復してループするのを防ぐ
    suppressScrollSignal = true;
    target->setValue(static_cast<int>(std::lround(ratio * target->maximum())));
    suppressScrollSignal = false;
}
