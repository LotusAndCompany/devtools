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
    formatSelector = ui->formatSelector;
    styleSelector = ui->styleSelector;

    if (dataConversion->parent() == nullptr)
        dataConversion->setParent(this);

    connect(ui->inputTextEdit,
            &QTextEdit::textChanged,
            this,
            &DataConversionGUI::onInputTextChanged);
    connect(ui->formatSelector,
            &QComboBox::currentIndexChanged,
            this,
            &DataConversionGUI::onFormatSelected);
    connect(ui->styleSelector,
            &QComboBox::currentIndexChanged,
            this,
            &DataConversionGUI::onStyleSelected);
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
    dataConversion->updateOutputText();
    outputTextView->setPlainText(dataConversion->outputText());
}

void DataConversionGUI::onFormatSelected(int index)
{
    switch (index) {
    case 0:
        dataConversion->setOutputFormat(DataConversion::Format::JSON);
        break;
    case 1:
        dataConversion->setOutputFormat(DataConversion::Format::YAML_BLOCK);
        break;
    case 2:
        dataConversion->setOutputFormat(DataConversion::Format::YAML_FLOW);
        break;
    case 3:
        dataConversion->setOutputFormat(DataConversion::Format::TOML);
        break;
    }
    dataConversion->updateOutputText();
    outputTextView->setPlainText(dataConversion->outputText());
}

void DataConversionGUI::onStyleSelected(int index)
{
    switch (index) {
    case 0:
        dataConversion->setIndentation(DataConversion::Indentation::SPACES_4);
        break;
    case 1:
        dataConversion->setIndentation(DataConversion::Indentation::SPACES_2);
        break;
    case 2:
        dataConversion->setIndentation(DataConversion::Indentation::TABS);
        break;
    case 3:
        dataConversion->setIndentation(DataConversion::Indentation::MINIFIED);
        break;
    }
    dataConversion->updateOutputText();
    outputTextView->setPlainText(dataConversion->outputText());
}
