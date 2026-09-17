#ifndef MARKDOWN_PREVIEW_GUI_H
#define MARKDOWN_PREVIEW_GUI_H

#include "features/framework/gui/gui_tool.h"

#include <QString>

class MarkdownPreview;

class QAbstractSlider;
class QCheckBox;
class QEvent;
class QGroupBox;
class QPlainTextEdit;
class QPushButton;
class QTextBrowser;
class QTimer;
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
    QGroupBox *buildPane(QWidget *content);

    MarkdownPreview *tool;

    QGroupBox *toolbarGroupBox{nullptr};
    QPushButton *openButton{nullptr};
    QPushButton *saveButton{nullptr};
    QPushButton *exportHtmlButton{nullptr};
    QCheckBox *syncScrollCheck{nullptr};
    QGroupBox *editorPane{nullptr};
    QGroupBox *previewPane{nullptr};
    QPlainTextEdit *editor{nullptr};
    QTextBrowser *preview{nullptr};
    QTimer *renderTimer{nullptr};

    bool syncScrollEnabled{true};
    bool suppressScrollSignal{false};
    QString currentFilePath;
};

#endif // MARKDOWN_PREVIEW_GUI_H
