#include "data_conversion_gui.h"

#include "features/framework/gui/design_system.h"

#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStandardPaths>
#include <QVBoxLayout>

DataConversionGUI::DataConversionGUI(DataConversionInterface *dataConversion, QWidget *parent)
    : GuiTool(parent), dataConversion(dataConversion)
{
    buildUi();

    if (dataConversion->parent() == nullptr) {
        dataConversion->setParent(this);
    }

    connect(inputTextEdit, &QPlainTextEdit::textChanged, this,
            &DataConversionGUI::onInputTextChanged);
    connect(loadButton, &QPushButton::pressed, this, &DataConversionGUI::onLoadPressed);
    connect(pasteButton, &QPushButton::pressed, this, &DataConversionGUI::onPastePressed);
    connect(clearButton, &QPushButton::pressed, this, &DataConversionGUI::onClearPressed);

    connect(formatSelector, &QComboBox::currentIndexChanged, this,
            &DataConversionGUI::onFormatSelected);
    connect(styleSelector, &QComboBox::currentIndexChanged, this,
            &DataConversionGUI::onStyleSelected);
    connect(copyButton, &QPushButton::pressed, this, &DataConversionGUI::onCopyPressed);
    connect(saveButton, &QPushButton::pressed, this, &DataConversionGUI::onSavePressed);
}

void DataConversionGUI::buildUi()
{
    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);

    splitter->addWidget(buildInputSide(splitter));
    splitter->addWidget(buildOutputSide(splitter));
    DevTools::Ui::configureEqualSplitter(splitter);

    auto *const rootLayout = new QHBoxLayout(this);
    DevTools::Ui::applyPageLayout(rootLayout);
    rootLayout->addWidget(splitter);
}

QWidget *DataConversionGUI::buildInputSide(QWidget *parent)
{
    auto *const container = new QGroupBox(parent);
    container->setTitle(tr("Input"));
    auto *const layout = new QVBoxLayout(container);
    DevTools::Ui::applyPanelLayout(layout);

    input_action_button_layout = new QHBoxLayout;
    DevTools::Ui::applyToolbarLayout(input_action_button_layout);

    loadButton = new QPushButton(tr("Load"), container);
    DevTools::Ui::configureCompactButton(loadButton);
    loadButton->setIcon(QIcon::fromTheme(QStringLiteral("file")));
    input_action_button_layout->addWidget(loadButton);

    pasteButton = new QPushButton(tr("Paste"), container);
    DevTools::Ui::configureCompactButton(pasteButton);
    pasteButton->setIcon(QIcon::fromTheme(QStringLiteral("content_paste")));
    input_action_button_layout->addWidget(pasteButton);

    input_action_button_layout->addStretch();

    clearButton = new QPushButton(container);
    DevTools::Ui::configureIconButton(clearButton, QStringLiteral("close"), tr("Clear"));
    input_action_button_layout->addWidget(clearButton);

    layout->addLayout(input_action_button_layout);

    inputTextEdit = new QPlainTextEdit(container);
    DevTools::Ui::configureCodeEditor(inputTextEdit);
    inputTextEdit->setPlaceholderText(tr("Input text"));
    layout->addWidget(inputTextEdit);

    inputMessageTextView = new QPlainTextEdit(container);
    DevTools::Ui::configureStatusView(inputMessageTextView);
    inputMessageTextView->setPlaceholderText(tr("Error & warning messages"));
    layout->addWidget(inputMessageTextView);

    return container;
}

QWidget *DataConversionGUI::buildOutputSide(QWidget *parent)
{
    auto *const container = new QGroupBox(parent);
    container->setTitle(tr("Output"));
    auto *const layout = new QVBoxLayout(container);
    DevTools::Ui::applyPanelLayout(layout);

    output_action_button_layout = new QHBoxLayout;
    DevTools::Ui::applyToolbarLayout(output_action_button_layout);

    formatSelector = new QComboBox(container);
    formatSelector->addItem(QStringLiteral("JSON"));
    formatSelector->addItem(tr("YAML (Block style)"));
    formatSelector->addItem(tr("YAML (Flow style)"));
    formatSelector->addItem(QStringLiteral("TOML"));
    output_action_button_layout->addWidget(formatSelector);

    styleSelector = new QComboBox(container);
    styleSelector->addItem(tr("4 Spaces"));
    styleSelector->addItem(tr("2 Spaces"));
    styleSelector->addItem(tr("Tabs"));
    styleSelector->addItem(tr("Minified"));
    output_action_button_layout->addWidget(styleSelector);

    output_action_button_layout->addStretch();

    saveButton = new QPushButton(tr("Save"), container);
    DevTools::Ui::configureCompactButton(saveButton);
    saveButton->setIcon(QIcon::fromTheme(QStringLiteral("save")));
    output_action_button_layout->addWidget(saveButton);

    copyButton = new QPushButton(tr("Copy"), container);
    DevTools::Ui::configureCompactButton(copyButton);
    copyButton->setIcon(QIcon::fromTheme(QStringLiteral("content_copy")));
    output_action_button_layout->addWidget(copyButton);

    layout->addLayout(output_action_button_layout);

    outputTextView = new QPlainTextEdit(container);
    DevTools::Ui::configureCodeEditor(outputTextView);
    outputTextView->setReadOnly(true);
    outputTextView->setTextInteractionFlags(Qt::TextSelectableByKeyboard |
                                            Qt::TextSelectableByMouse);
    outputTextView->setPlaceholderText(tr("Output text"));
    layout->addWidget(outputTextView);

    outputMessageTextView = new QPlainTextEdit(container);
    DevTools::Ui::configureStatusView(outputMessageTextView);
    outputMessageTextView->setPlaceholderText(tr("Error & warning messages"));
    layout->addWidget(outputMessageTextView);

    return container;
}

void DataConversionGUI::onInputTextChanged()
{
    // TODO: 処理が重くなるため、呼び出し頻度に制限を設ける
    dataConversion->setInputText(inputTextEdit->toPlainText());
    inputMessageTextView->setPlainText(dataConversion->messages());
    dataConversion->updateOutputText();
    outputMessageTextView->setPlainText(dataConversion->messages());
    outputTextView->setPlainText(dataConversion->outputText());
}

void DataConversionGUI::onPastePressed()
{
    QClipboard const *const clipboard = QGuiApplication::clipboard();
    inputTextEdit->setPlainText(clipboard->text()); // onInputTextChanged()
}

void DataConversionGUI::onClearPressed()
{
    inputTextEdit->setPlainText("");
    dataConversion->setInputText("");
    inputMessageTextView->setPlainText("");
    dataConversion->updateOutputText();
    outputMessageTextView->setPlainText("");
    outputTextView->setPlainText("");
}

void DataConversionGUI::onLoadPressed()
{
    QFileDialog dialog(this);
    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Plain Text (*.txt *.json *.toml *.yml *yaml)");
    connect(&dialog, &QFileDialog::fileSelected, this, [this](const QString &fileName) {
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
    default:
        break;
    }
    dataConversion->updateOutputText();
    outputMessageTextView->setPlainText(dataConversion->messages());
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
    default:
        break;
    }
    dataConversion->updateOutputText();
    outputMessageTextView->setPlainText(dataConversion->messages());
    outputTextView->setPlainText(dataConversion->outputText());
}

void DataConversionGUI::onSavePressed()
{
    QFileDialog dialog(this);
    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    connect(&dialog, &QFileDialog::fileSelected, this, [this](const QString &fileName) {
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
    if (text != "") {
        clipboard->setText(text);
    }
}
