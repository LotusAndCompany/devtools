#ifndef DATA_CONVERSION_GUI_H
#define DATA_CONVERSION_GUI_H

#include <QWidget>

#include "core/data_conversion/data_conversion.h"
#include "gui/gui_tool.h"

namespace Ui {
class DataConversionGUI;
}

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

private:
    Ui::DataConversionGUI *const ui;

    /// ロジック部分
    DataConversionInterface *const dataConversion;

private slots:
    void onInputTextChanged();
    void onPastePressed();
    void onLoadPressed();
    void onClearPressed();
    void onSavePressed();
    void onCopyPressed();
};

#endif // DATA_CONVERSION_GUI_H
