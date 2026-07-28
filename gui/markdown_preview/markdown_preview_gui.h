#ifndef MARKDOWN_PREVIEW_GUI_H
#define MARKDOWN_PREVIEW_GUI_H

#include "gui/gui_tool.h"

#include <QString>

class MarkdownPreview;

class QAbstractSlider;
class QAction;
class QCheckBox;
class QEvent;
class QLabel;
class QPlainTextEdit;
class QTextBrowser;
class QTimer;
class QToolBar;
class QWidget;

/**
 * @brief Markdown プレビューの GUI
 * @details 左ペイン(エディタ) / 右ペイン(プレビュー)のリアルタイムプレビュー。@n
 *          `.ui` は使わずコンストラクタでウィジェットを構築する。
 */
class MarkdownPreviewGUI : public GuiTool
{
    Q_OBJECT
    Q_DISABLE_COPY(MarkdownPreviewGUI)

public:
    explicit MarkdownPreviewGUI(MarkdownPreview *tool, QWidget *parent = nullptr);
    ~MarkdownPreviewGUI() override = default;
    MarkdownPreviewGUI(MarkdownPreviewGUI &&) = delete;
    MarkdownPreviewGUI &operator=(MarkdownPreviewGUI &&) = delete;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onOpenClicked();
    void onSaveClicked();
    void onExportHtmlClicked();
    void onSyncScrollToggled(bool enabled);
    void onEditorTextChanged();
    void onRenderTimeout();
    void onEditorScrolled();
    void onPreviewScrolled();

private:
    void buildUi();
    void retranslateUi();
    void renderPreview();
    void syncScroll(QAbstractSlider *source, QAbstractSlider *target);
    QWidget *buildPane(QLabel *&sectionLabel, QWidget *content);

    MarkdownPreview *tool;

    QToolBar *toolbar{nullptr};
    QAction *openAction{nullptr};
    QAction *saveAction{nullptr};
    QAction *exportHtmlAction{nullptr};
    QCheckBox *syncScrollCheck{nullptr};
    QLabel *editorLabel{nullptr};
    QLabel *previewLabel{nullptr};
    QPlainTextEdit *editor{nullptr};
    QTextBrowser *preview{nullptr};
    QTimer *renderTimer{nullptr};

    bool syncScrollEnabled{true};
    bool suppressScrollSignal{false};
    QString currentFilePath;
};

#endif // MARKDOWN_PREVIEW_GUI_H
