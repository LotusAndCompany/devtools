#include "data_conversion_gui.h"
#include "ui_data_conversion_gui.h"

#include <QClipboard>
#include <QResizeEvent>
#include <cmath>

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
    connect(ui->pasteButton, &QPushButton::pressed, this, &DataConversionGUI::onPastePressed);
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
    const int width = ui->inputActionButtonLayout->minimumSize().width()
                      + ui->outputActionButtonLayout->minimumSize().width();
    ui->splitter->setMinimumWidth(width);
    setMinimumWidth(width);
    /*
    if (ui->inputActionButtonLayout->minimumSize().height()
        < ui->outputActionButtonLayout->minimumSize().height())
        ui->inputActionButtonLayout*/

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
    dataConversion->setInputText(inputTextEdit->toPlainText());
    ui->inputMessageTextView->setText(dataConversion->messages());
    dataConversion->updateOutputText();
    ui->outputMessageTextView->setText(dataConversion->messages());
    outputTextView->setPlainText(dataConversion->outputText());
}

void DataConversionGUI::onPastePressed()
{
    QClipboard *const clipboard = QGuiApplication::clipboard();
    inputTextEdit->setText(clipboard->text()); // onInputTextChanged()
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
    ui->outputMessageTextView->setText(dataConversion->messages());
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
    ui->outputMessageTextView->setText(dataConversion->messages());
    outputTextView->setPlainText(dataConversion->outputText());
}

void DataConversionGUI::onClearPressed()
{
    dataConversion->setInputText("");
    ui->inputMessageTextView->setText("");
    dataConversion->updateOutputText();
    ui->outputMessageTextView->setText("");
    outputTextView->setPlainText("");
}
