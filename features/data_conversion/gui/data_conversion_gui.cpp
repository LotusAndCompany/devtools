#include "data_conversion_gui.h"

#include "features/framework/gui/icon_utils.h"

#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QResizeEvent>
#include <QSpacerItem>
#include <QSplitter>
#include <QStandardPaths>
#include <QStyle>
#include <QTextBrowser>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {
constexpr int MESSAGE_VIEW_MAX_HEIGHT = 64;
constexpr int CLEAR_BUTTON_SIZE = 34;
constexpr int CLEAR_BUTTON_ICON_SIZE = 16;
constexpr int ACTION_BAR_MIN_HEIGHT = 38;
constexpr qreal TAB_STOP_DISTANCE = 20.0;
constexpr int DEFAULT_WIDTH = 715;
constexpr int DEFAULT_HEIGHT = 256;
} // namespace

DataConversionGUI::DataConversionGUI(DataConversionInterface *dataConversion, QWidget *parent)
    : GuiTool(parent), dataConversion(dataConversion)
{
    buildUi();

    if (dataConversion->parent() == nullptr) {
        dataConversion->setParent(this);
    }

    connect(inputTextEdit, &QTextEdit::textChanged, this, &DataConversionGUI::onInputTextChanged);
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
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);
    splitter->setGeometry(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);

    splitter->addWidget(buildInputSide(splitter));
    splitter->addWidget(buildOutputSide(splitter));
}

QWidget *DataConversionGUI::buildInputSide(QWidget *parent)
{
    auto *const container = new QWidget(parent);
    auto *const layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *const actionBar = new QWidget(container);
    {
        QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        policy.setHorizontalStretch(0);
        policy.setVerticalStretch(0);
        actionBar->setSizePolicy(policy);
    }
    actionBar->setMinimumHeight(ACTION_BAR_MIN_HEIGHT);

    input_action_button_layout = new QHBoxLayout(actionBar);
    input_action_button_layout->setContentsMargins(4, 0, 4, 0);

    loadButton = new QPushButton(tr("Load"), actionBar);
    loadButton->setIcon(
        IconUtils::themedIcon(QStringLiteral("file_open"), QStyle::SP_DialogOpenButton));
    input_action_button_layout->addWidget(loadButton);

    pasteButton = new QPushButton(tr("Paste"), actionBar);
    pasteButton->setIcon(IconUtils::themedIcon(QStringLiteral("content_paste")));
    input_action_button_layout->addWidget(pasteButton);

    input_action_button_layout->addItem(
        new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    clearButton = new QPushButton(actionBar);
    {
        QSizePolicy policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        policy.setHorizontalStretch(0);
        policy.setVerticalStretch(0);
        clearButton->setSizePolicy(policy);
    }
    clearButton->setMinimumSize(CLEAR_BUTTON_SIZE, CLEAR_BUTTON_SIZE);
    clearButton->setMaximumSize(CLEAR_BUTTON_SIZE, CLEAR_BUTTON_SIZE);
    clearButton->setAutoFillBackground(false);
    clearButton->setIcon(
        IconUtils::themedIcon(QStringLiteral("close"), QStyle::SP_DialogCloseButton));
    clearButton->setIconSize(QSize(CLEAR_BUTTON_ICON_SIZE, CLEAR_BUTTON_ICON_SIZE));
    clearButton->setFlat(false);
    input_action_button_layout->addWidget(clearButton);

    layout->addWidget(actionBar);

    inputTextEdit = new QTextEdit(container);
    inputTextEdit->setTabStopDistance(TAB_STOP_DISTANCE);
    inputTextEdit->setAcceptRichText(false);
    inputTextEdit->setPlaceholderText(tr("Input text"));
    layout->addWidget(inputTextEdit);

    inputMessageTextView = new QTextBrowser(container);
    inputMessageTextView->setMaximumHeight(MESSAGE_VIEW_MAX_HEIGHT);
    inputMessageTextView->setTextInteractionFlags(Qt::TextSelectableByKeyboard |
                                                  Qt::TextSelectableByMouse);
    inputMessageTextView->setPlaceholderText(tr("Error & waning messages"));
    layout->addWidget(inputMessageTextView);

    return container;
}

QWidget *DataConversionGUI::buildOutputSide(QWidget *parent)
{
    auto *const container = new QWidget(parent);
    auto *const layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *const actionBar = new QWidget(container);
    output_action_button_layout = new QHBoxLayout(actionBar);
    output_action_button_layout->setContentsMargins(4, 0, 4, 0);

    formatSelector = new QComboBox(actionBar);
    formatSelector->addItem(QStringLiteral("JSON"));
    formatSelector->addItem(tr("YAML (Block style)"));
    formatSelector->addItem(tr("YAML (Flow style)"));
    formatSelector->addItem(QStringLiteral("TOML"));
    output_action_button_layout->addWidget(formatSelector);

    styleSelector = new QComboBox(actionBar);
    styleSelector->addItem(tr("4 Spaces"));
    styleSelector->addItem(tr("2 Spaces"));
    styleSelector->addItem(tr("Tabs"));
    styleSelector->addItem(tr("Minified"));
    output_action_button_layout->addWidget(styleSelector);

    output_action_button_layout->addItem(
        new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    saveButton = new QPushButton(tr("Save"), actionBar);
    saveButton->setIcon(IconUtils::themedIcon(QStringLiteral("save"), QStyle::SP_DialogSaveButton));
    output_action_button_layout->addWidget(saveButton);

    copyButton = new QPushButton(tr("Copy"), actionBar);
    copyButton->setIcon(IconUtils::themedIcon(QStringLiteral("content_copy")));
    output_action_button_layout->addWidget(copyButton);

    layout->addWidget(actionBar);

    outputTextView = new QTextBrowser(container);
    outputTextView->setTabStopDistance(TAB_STOP_DISTANCE);
    outputTextView->setTextInteractionFlags(Qt::TextSelectableByKeyboard |
                                            Qt::TextSelectableByMouse);
    outputTextView->setPlaceholderText(tr("Output text"));
    outputTextView->setOpenLinks(false);
    layout->addWidget(outputTextView);

    outputMessageTextView = new QTextBrowser(container);
    outputMessageTextView->setMaximumHeight(MESSAGE_VIEW_MAX_HEIGHT);
    outputMessageTextView->setTextInteractionFlags(Qt::TextSelectableByKeyboard |
                                                   Qt::TextSelectableByMouse);
    outputMessageTextView->setPlaceholderText(tr("Error & waning messages"));
    layout->addWidget(outputMessageTextView);

    return container;
}

void DataConversionGUI::resizeEvent(QResizeEvent *event)
{
    // NOTE: ここで最小幅を設定する
    const int width = input_action_button_layout->minimumSize().width() +
                      output_action_button_layout->minimumSize().width();
    splitter->setMinimumWidth(width);
    setMinimumWidth(width);

    QSize size = event->size();
    if (size.width() < splitter->minimumSizeHint().width()) {
        size.setWidth(splitter->minimumSizeHint().width());
    }
    if (size.height() < splitter->minimumSizeHint().height()) {
        size.setHeight(splitter->minimumSizeHint().height());
    }

    splitter->resize(size);
    event->accept();
}

void DataConversionGUI::onInputTextChanged()
{
    // TODO: 処理が重くなるため、呼び出し頻度に制限を設ける
    dataConversion->setInputText(inputTextEdit->toPlainText());
    inputMessageTextView->setText(dataConversion->messages());
    dataConversion->updateOutputText();
    outputMessageTextView->setText(dataConversion->messages());
    outputTextView->setPlainText(dataConversion->outputText());
}

void DataConversionGUI::onPastePressed()
{
    QClipboard const *const clipboard = QGuiApplication::clipboard();
    inputTextEdit->setText(clipboard->text()); // onInputTextChanged()
}

void DataConversionGUI::onClearPressed()
{
    inputTextEdit->setText("");
    dataConversion->setInputText("");
    inputMessageTextView->setText("");
    dataConversion->updateOutputText();
    outputMessageTextView->setText("");
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
    outputMessageTextView->setText(dataConversion->messages());
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
    outputMessageTextView->setText(dataConversion->messages());
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
