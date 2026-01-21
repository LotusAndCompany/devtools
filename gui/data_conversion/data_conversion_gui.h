#ifndef DATA_CONVERSION_GUI_H
#define DATA_CONVERSION_GUI_H

#include "core/data_conversion/data_conversion.h"
#include "gui/gui_tool.h"

#include <QWidget>

namespace Ui {
class DataConversionGUI;
}

class QTextEdit;
class QTextBrowser;
class QComboBox;
class QPushButton;

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
    ~DataConversionGUI();

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
    Ui::DataConversionGUI *const ui;

    /// ロジック部分
    DataConversionInterface *const dataConversion;

    /// テキスト入力UI
    QTextEdit *inputTextEdit;
    /// 貼り付けボタン
    QPushButton *pasteButton;
    /// 読み込みボタン
    QPushButton *loadButton;
    /// 消去ボタン
    QPushButton *clearButton;

    /// テキスト表示UI
    QTextBrowser *outputTextView;
    /// 出力フォーマット選択UI
    QComboBox *formatSelector;
    /// インデント選択UI
    QComboBox *styleSelector;
    /// コピーボタン
    QPushButton *copyButton;
    /// 保存ボタン
    QPushButton *saveButton;

#ifdef _TEST_ImageDivisionGUI
    frientd class Test::TestDataConversionGUI;
#endif
};

#endif // DATA_CONVERSION_GUI_H
