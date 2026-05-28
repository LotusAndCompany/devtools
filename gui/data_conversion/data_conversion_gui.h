#ifndef DATA_CONVERSION_GUI_H
#define DATA_CONVERSION_GUI_H

#include "core/data_conversion/data_conversion.h"
#include "gui/gui_tool.h"

#include <QWidget>

class QComboBox;
class QHBoxLayout;
class QPushButton;
class QSplitter;
class QTextBrowser;
class QTextEdit;

#ifdef _TEST_ImageDivisionGUI
namespace Test {
class TestDataConversionGUI;
}
#endif

/// シリアライズされたデータの変換、フォーマットを行うツールのGUI
class DataConversionGUI : public GuiTool
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param dataConversion ロジック部分
     * @param parent 親ウィジェット
     */
    explicit DataConversionGUI(DataConversionInterface *dataConversion, QWidget *parent = nullptr);
    /// デストラクタ
    ~DataConversionGUI() override = default;
    DataConversionGUI(const DataConversionGUI &) = delete;
    DataConversionGUI &operator=(const DataConversionGUI &) = delete;
    DataConversionGUI(DataConversionGUI &&) = delete;
    DataConversionGUI &operator=(DataConversionGUI &&) = delete;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onInputTextChanged();
    void onFormatSelected(int index);
    void onStyleSelected(int index);
    void onPastePressed();
    void onClearPressed();
    void onLoadPressed();
    void onSavePressed();
    void onCopyPressed();

private:
    /// UIを構築する
    void buildUi();
    /// 入力側のUIを構築する
    QWidget *buildInputSide(QWidget *parent);
    /// 出力側のUIを構築する
    QWidget *buildOutputSide(QWidget *parent);

    /// ロジック部分
    DataConversionInterface *const dataConversion;

    /// メインスプリッタ
    QSplitter *splitter = nullptr;

    /// 入力側アクションボタン行のレイアウト
    QHBoxLayout *input_action_button_layout = nullptr;
    /// 出力側アクションボタン行のレイアウト
    QHBoxLayout *output_action_button_layout = nullptr;

    /// テキスト入力UI
    QTextEdit *inputTextEdit = nullptr;
    /// 入力側のエラー・警告メッセージ表示UI
    QTextBrowser *inputMessageTextView = nullptr;
    /// 貼り付けボタン
    QPushButton *pasteButton = nullptr;
    /// 読み込みボタン
    QPushButton *loadButton = nullptr;
    /// 消去ボタン
    QPushButton *clearButton = nullptr;

    /// テキスト表示UI
    QTextBrowser *outputTextView = nullptr;
    /// 出力側のエラー・警告メッセージ表示UI
    QTextBrowser *outputMessageTextView = nullptr;
    /// 出力フォーマット選択UI
    QComboBox *formatSelector = nullptr;
    /// インデント選択UI
    QComboBox *styleSelector = nullptr;
    /// コピーボタン
    QPushButton *copyButton = nullptr;
    /// 保存ボタン
    QPushButton *saveButton = nullptr;

#ifdef _TEST_ImageDivisionGUI
    frientd class Test::TestDataConversionGUI;
#endif
};

#endif // DATA_CONVERSION_GUI_H
