#include "data_conversion_gui.h"
#include "ui_data_conversion_gui.h"

#include <QClipboard>
#include <QFileDialog>
#include <QResizeEvent>
#include <QStandardPaths>

DataConversionGUI::DataConversionGUI(DataConversionInterface *dataConversion, QWidget *parent)
    : GuiTool(parent)
    , dataConversion(dataConversion)
    , ui(new Ui::DataConversionGUI)
{
    ui->setupUi(this);

    inputTextEdit = ui->inputTextEdit;
    pasteButton = ui->pasteButton;
    loadButton = ui->loadButton;
    clearButton = ui->clearButton;

    outputTextView = ui->outputTextView;
    formatSelector = ui->formatSelector;
    styleSelector = ui->styleSelector;
    copyButton = ui->copyButton;
    saveButton = ui->saveButton;

    if (dataConversion->parent() == nullptr)
        dataConversion->setParent(this);

    connect(inputTextEdit, &QTextEdit::textChanged, this, &DataConversionGUI::onInputTextChanged);
    connect(loadButton, &QPushButton::pressed, this, &DataConversionGUI::onLoadPressed);
    connect(pasteButton, &QPushButton::pressed, this, &DataConversionGUI::onPastePressed);
    connect(clearButton, &QPushButton::pressed, this, &DataConversionGUI::onClearPressed);

    connect(formatSelector,
            &QComboBox::currentIndexChanged,
            this,
            &DataConversionGUI::onFormatSelected);
    connect(styleSelector,
            &QComboBox::currentIndexChanged,
            this,
            &DataConversionGUI::onStyleSelected);
    connect(copyButton, &QPushButton::pressed, this, &DataConversionGUI::onCopyPressed);
    connect(saveButton, &QPushButton::pressed, this, &DataConversionGUI::onSavePressed);
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
    // TODO: 処理が重くなるため、呼び出し頻度に制限を設ける
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

void DataConversionGUI::onClearPressed()
{
    inputTextEdit->setText("");
    dataConversion->setInputText("");
    ui->inputMessageTextView->setText("");
    dataConversion->updateOutputText();
    ui->outputMessageTextView->setText("");
    outputTextView->setPlainText("");
}

void DataConversionGUI::onLoadPressed()
{
    QFileDialog dialog(this);
    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Plain Text (*.txt *.json *.toml *.yml *yaml)");
    connect(&dialog, &QFileDialog::fileSelected, this, [=](const QString &fileName) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            const QString text = stream.readAll();

            inputTextEdit->setPlainText(text);
        }
    });
    dialog.exec();
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

void DataConversionGUI::onSavePressed()
{
    QFileDialog dialog(this);
    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    connect(&dialog, &QFileDialog::fileSelected, this, [=](const QString &fileName) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << outputTextView->toPlainText();
        }
    });
    dialog.exec();
}

void DataConversionGUI::onCopyPressed()
{
    QClipboard *const clipboard = QGuiApplication::clipboard();
    const auto text = outputTextView->toPlainText();
    if (text != "")
        clipboard->setText(text);
}
