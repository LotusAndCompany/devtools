#include "markdown_preview_gui.h"

#include "features/markdown_preview/core/markdown_preview.h"

#include <QAbstractSlider>
#include <QAction>
#include <QCheckBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QMessageBox>
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

#include <oclero/qlementine/widgets/Label.hpp>

namespace {
constexpr int RENDER_DEBOUNCE_MS = 180;
constexpr int DEFAULT_WIDTH = 800;
constexpr int DEFAULT_HEIGHT = 600;
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
    // 枠 (角丸のテキストフィールド) は qlementine スタイルに描画させる。
    // .pen のエディタは等幅フォント。インストール環境に依存しないよう
    // システム標準の等幅フォントを使う。
    const QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    editor->setFont(monoFont);

    preview = new QTextBrowser;
    preview->setOpenExternalLinks(true);
    // レンダリング結果が等幅にならないよう、本文は UI 標準フォントにする。
    preview->setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    auto *editorPane = buildPane(editorLabel, editor);
    auto *previewPane = buildPane(previewLabel, preview);

    splitter->addWidget(editorPane);
    splitter->addWidget(previewPane);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter);

    renderTimer = new QTimer(this);
    renderTimer->setSingleShot(true);
    renderTimer->setInterval(RENDER_DEBOUNCE_MS);

    retranslateUi();
}

QWidget *MarkdownPreviewGUI::buildPane(QLabel *&sectionLabel, QWidget *content)
{
    auto *pane = new QWidget(this);

    auto *layout = new QVBoxLayout(pane);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    // セクション見出しは qlementine の Caption ロールでテーマに追従させる。
    sectionLabel =
        new oclero::qlementine::Label(QString(), oclero::qlementine::TextRole::Caption, pane);

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
