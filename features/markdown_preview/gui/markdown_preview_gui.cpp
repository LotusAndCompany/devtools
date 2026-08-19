#include "markdown_preview_gui.h"

#include "features/framework/gui/design_system.h"
#include "features/markdown_preview/core/markdown_preview.h"

#include <QAbstractSlider>
#include <QCheckBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
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
} // namespace

MarkdownPreviewGUI::MarkdownPreviewGUI(MarkdownPreview *tool, QWidget *parent)
    : GuiTool(parent), tool(tool)
{
    buildUi();

    // NOTE: parentが設定されていなければこのインスタンスで管理する
    if (tool != nullptr && tool->parent() == nullptr) {
        tool->setParent(this);
    }

    connect(openButton, &QPushButton::clicked, this, &MarkdownPreviewGUI::onOpenClicked);
    connect(saveButton, &QPushButton::clicked, this, &MarkdownPreviewGUI::onSaveClicked);
    connect(exportHtmlButton, &QPushButton::clicked, this,
            &MarkdownPreviewGUI::onExportHtmlClicked);
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
    auto *mainLayout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(mainLayout);

    toolbarGroupBox = new QGroupBox(this);
    toolbarGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *toolbarLayout = new QHBoxLayout(toolbarGroupBox);
    DevTools::Ui::applyPanelLayout(toolbarLayout);

    openButton = new QPushButton(toolbarGroupBox);
    saveButton = new QPushButton(toolbarGroupBox);
    exportHtmlButton = new QPushButton(toolbarGroupBox);
    for (auto *button : {openButton, saveButton, exportHtmlButton}) {
        DevTools::Ui::configureCompactButton(button);
        toolbarLayout->addWidget(button);
    }

    toolbarLayout->addStretch();

    syncScrollCheck = new QCheckBox(toolbarGroupBox);
    syncScrollCheck->setChecked(syncScrollEnabled);
    toolbarLayout->addWidget(syncScrollCheck);
    mainLayout->addWidget(toolbarGroupBox);

    auto *splitter = new QSplitter(Qt::Horizontal, this);

    editor = DevTools::Ui::createPlainTextEdit(splitter);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    // 枠 (角丸のテキストフィールド) は qlementine スタイルに描画させる。
    // .pen のエディタは等幅フォント。インストール環境に依存しないよう
    // システム標準の等幅フォントを使う。
    DevTools::Ui::configureCodeEditor(editor);

    preview = DevTools::Ui::createTextBrowser();
    preview->setOpenExternalLinks(true);
    DevTools::Ui::configureDocumentPreview(preview);

    editorPane = buildPane(editor);
    previewPane = buildPane(preview);

    splitter->addWidget(editorPane);
    splitter->addWidget(previewPane);
    DevTools::Ui::configureEqualSplitter(splitter);
    mainLayout->addWidget(splitter, 1);

    renderTimer = new QTimer(this);
    renderTimer->setSingleShot(true);
    renderTimer->setInterval(RENDER_DEBOUNCE_MS);

    retranslateUi();
}

QGroupBox *MarkdownPreviewGUI::buildPane(QWidget *content)
{
    auto *pane = new QGroupBox(this);

    auto *layout = new QVBoxLayout(pane);
    DevTools::Ui::applyPanelLayout(layout);
    layout->addWidget(content);

    return pane;
}

void MarkdownPreviewGUI::retranslateUi()
{
    setWindowTitle(tr("Markdown Preview"));
    toolbarGroupBox->setTitle(tr("Toolbar"));
    toolbarGroupBox->setFixedHeight(76);
    openButton->setText(tr("Open"));
    saveButton->setText(tr("Save"));
    exportHtmlButton->setText(tr("Export HTML"));
    syncScrollCheck->setText(tr("Sync scroll"));
    editorPane->setTitle(tr("Editor"));
    previewPane->setTitle(tr("Preview"));
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
