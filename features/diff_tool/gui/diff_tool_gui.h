#ifndef DIFF_TOOL_GUI_H
#define DIFF_TOOL_GUI_H

#include "features/diff_tool/core/diff_tool.h"
#include "features/framework/gui/gui_tool.h"

#include <QString>
#include <QVector>

class DiffTool;

class QComboBox;
class QEvent;
class QGroupBox;
class QPlainTextEdit;
class QTextBrowser;
class QTimer;
class QWidget;

/**
 * @brief テキスト差分ツールの GUI
 * @details 左(変更前) / 右(変更後) の2つのテキストを入力し、差分をインラインまたは
 *          サイドバイサイドで表示する。`.ui` は使わずコンストラクタで構築する。
 */
class DiffToolGUI : public GuiTool
{
    Q_OBJECT
    Q_DISABLE_COPY(DiffToolGUI)

public:
    explicit DiffToolGUI(DiffTool *tool, QWidget *parent = nullptr);
    /// @brief インスタンスと QObject の親子関係で所有する子オブジェクトを破棄する
    ~DiffToolGUI() override = default;
    DiffToolGUI(DiffToolGUI &&) = delete;
    DiffToolGUI &operator=(DiffToolGUI &&) = delete;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onTextChanged();
    void onModeChanged(int index);
    void onRenderTimeout();

private:
    void buildUi();
    void retranslateUi();
    void renderDiff();
    static QString buildStyleSheet();
    static QString buildInlineHtml(const QVector<DiffLine> &lines);
    static QString buildSideBySideHtml(const QVector<DiffLine> &lines);
    static QGroupBox *buildEditorPane(const QString &title, QPlainTextEdit *&editor,
                                      QWidget *parent);

    DiffTool *tool;

    QGroupBox *leftPane{nullptr};
    QGroupBox *rightPane{nullptr};
    QGroupBox *resultPane{nullptr};
    QComboBox *modeCombo{nullptr};
    QPlainTextEdit *leftEditor{nullptr};
    QPlainTextEdit *rightEditor{nullptr};
    QTextBrowser *resultView{nullptr};
    QTimer *renderTimer{nullptr};
};

#endif // DIFF_TOOL_GUI_H
