#include "data_conversion_gui.h"
#include "ui_data_conversion_gui.h"

#include <QResizeEvent>

DataConversionGUI::DataConversionGUI(DataConversionInterface *dataConversion, QWidget *parent)
    : GuiTool(parent)
    , dataConversion(dataConversion)
    , ui(new Ui::DataConversionGUI)
{
    ui->setupUi(this);

    inputTextEdit = ui->inputTextEdit;
    outputTextView = ui->outputTextView;

    if (dataConversion->parent() == nullptr)
        dataConversion->setParent(this);

    connect(ui->inputTextEdit,
            &QTextEdit::textChanged,
            this,
            &DataConversionGUI::onInputTextChanged);
}

DataConversionGUI::~DataConversionGUI()
{
    delete ui;
}

void DataConversionGUI::resizeEvent(QResizeEvent *event)
{
    // NOTE: ここで最小幅を設定する
    const int width = ui->horizontalLayout->minimumSize().width()
                      + ui->horizontalLayout_2->minimumSize().width();
    ui->splitter->setMinimumWidth(width);
    setMinimumWidth(width);

    QSize size = event->size();
    if (size.width() < ui->splitter->minimumSizeHint().width())
        size.setWidth(ui->splitter->minimumSizeHint().width());
    if (size.height() < ui->splitter->minimumSizeHint().height())
        size.setHeight(ui->splitter->minimumSizeHint().height());

    ui->splitter->resize(size);
    event->accept();
}

void DataConversionGUI::onInputTextChanged()
{
    qDebug() << inputTextEdit->toPlainText();
    dataConversion->setInputText(inputTextEdit->toPlainText());
    outputTextView->setPlainText(dataConversion->outputText());
}
