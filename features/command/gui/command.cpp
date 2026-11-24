#include "command.h"

#include "features/command/core/command_service.h"
#include "features/framework/gui/design_system.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

Command::Command(QWidget *parent) : QWidget(parent)
{
    buildUi();
    init();

    connect(category_list, &QComboBox::currentIndexChanged, this, &Command::selectedCategory);
    connect(functions_list, &QComboBox::currentIndexChanged, this, &Command::selectedFunction);
    connect(option_list, &QComboBox::currentIndexChanged, this, &Command::selectedOption);
    connect(reset_button, &QPushButton::clicked, this, &Command::reset);
    connect(generate_button, &QPushButton::clicked, this, &Command::generate);
    connect(clear_button, &QPushButton::clicked, this, &Command::clear);
    connect(copy_button, &QPushButton::clicked, this, &Command::copy);
}

// NOLINTNEXTLINE(readability-function-size)
void Command::buildUi()
{
    setObjectName(QStringLiteral("Command"));

    auto *const layout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(layout);

    input_pane = DevTools::Ui::createPane(tr("Input"), this);
    DevTools::Ui::configureCompactPane(input_pane);
    input_pane->setObjectName(QStringLiteral("inputPane"));
    auto *const inputLayout = new QGridLayout(input_pane);
    DevTools::Ui::applyPanelLayout(inputLayout);

    category_list = new QComboBox(input_pane);
    DevTools::Ui::configureComboBox(category_list);
    category_list->setObjectName(QStringLiteral("categoryList"));
    inputLayout->addWidget(category_list, 0, 0, 1, 2);

    functions_label = new QLabel(tr("Functions List"), input_pane);
    functions_label->setObjectName(QStringLiteral("functionsLabel"));
    inputLayout->addWidget(functions_label, 1, 0);

    functions_list = new QComboBox(input_pane);
    DevTools::Ui::configureComboBox(functions_list);
    functions_list->setObjectName(QStringLiteral("functionsList"));
    functions_list->setEnabled(false);
    functions_list->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    inputLayout->addWidget(functions_list, 2, 0, 1, 2);

    option_label = new QLabel(tr("Option List"), input_pane);
    option_label->setObjectName(QStringLiteral("optionLabel"));
    inputLayout->addWidget(option_label, 3, 0);

    option_list = new QComboBox(input_pane);
    DevTools::Ui::configureComboBox(option_list);
    option_list->setObjectName(QStringLiteral("optionList"));
    option_list->setEnabled(false);
    inputLayout->addWidget(option_list, 4, 0, 1, 2);

    text_label = new QLabel(tr("TextLabel"), input_pane);
    text_label->setObjectName(QStringLiteral("label"));
    inputLayout->addWidget(text_label, 5, 0);

    text_edit = new QLineEdit(input_pane);
    DevTools::Ui::configureLineEdit(text_edit);
    text_edit->setObjectName(QStringLiteral("textEdit"));
    inputLayout->addWidget(text_edit, 6, 0, 1, 2);

    reset_button = new QPushButton(tr("Reset"), input_pane);
    reset_button->setObjectName(QStringLiteral("resetButton"));
    DevTools::Ui::configureCompactButton(reset_button);

    generate_button = new QPushButton(tr("Generate command"), input_pane);
    generate_button->setObjectName(QStringLiteral("generateButton"));
    DevTools::Ui::configureCompactButton(generate_button);

    auto *actionLayout = new QHBoxLayout;
    actionLayout->addWidget(reset_button);
    actionLayout->addWidget(generate_button);
    DevTools::Ui::configureActionBar(actionLayout, DevTools::Ui::ActionBarAlignment::Trailing);
    inputLayout->addLayout(actionLayout, 7, 0, 1, 2);

    layout->addWidget(input_pane);

    output_pane = DevTools::Ui::createPane(tr("Generated Result"), this);
    output_pane->setObjectName(QStringLiteral("outputPane"));
    auto *const outputLayout = new QGridLayout(output_pane);
    DevTools::Ui::applyPanelLayout(outputLayout);

    text_browser = new QPlainTextEdit(output_pane);
    text_browser->setObjectName(QStringLiteral("textBrowser"));
    DevTools::Ui::configureDisplayTextControl(text_browser);
    text_browser->setReadOnly(true);
    outputLayout->addWidget(text_browser, 0, 0, 1, 2);
    outputLayout->setRowStretch(0, 1);

    copy_button = new QPushButton(tr("Copy"), output_pane);
    copy_button->setObjectName(QStringLiteral("copyButton"));
    DevTools::Ui::configureCompactButton(copy_button);

    clear_button = new QPushButton(tr("Clear"), output_pane);
    clear_button->setObjectName(QStringLiteral("clearButton"));
    DevTools::Ui::configureCompactButton(clear_button);

    auto *outputActionLayout = new QHBoxLayout;
    outputActionLayout->addWidget(copy_button);
    outputActionLayout->addWidget(clear_button);
    DevTools::Ui::configureActionBar(outputActionLayout,
                                     DevTools::Ui::ActionBarAlignment::Trailing);
    outputLayout->addLayout(outputActionLayout, 1, 0, 1, 2);

    layout->addWidget(output_pane);
    layout->setStretch(1, DevTools::Ui::Metrics::MAIN_PANEL_STRETCH);
}

void Command::init()
{
    text_label->setVisible(false);
    text_edit->setVisible(false);

    functions_label->setVisible(false);
    functions_list->setVisible(false);
    option_label->setVisible(false);
    option_list->setVisible(false);

    retranslateUi();
}

void Command::retranslateUi()
{
    const int categoryIndex = category_list->currentIndex();
    const int functionIndex = functions_list->currentIndex();
    const int optionIndex = option_list->currentIndex();
    const QString inputValue = text_edit->text();
    const QString outputValue = text_browser->toPlainText();

    setWindowTitle(tr("Command Generation"));
    input_pane->setTitle(tr("Input"));
    output_pane->setTitle(tr("Generated Result"));
    functions_label->setText(tr("Functions List"));
    option_label->setText(tr("Option List"));
    reset_button->setText(tr("Reset"));
    generate_button->setText(tr("Generate command"));
    copy_button->setText(tr("Copy"));
    clear_button->setText(tr("Clear"));

    category_list->clear();
    category_list->addItems({tr("Categories"), tr("1: Git commands"), tr("2: Docker commands"),
                             tr("3: Docker Compose commands")});
    category_list->setCurrentIndex(categoryIndex >= 0 ? categoryIndex : 0);

    if (category_list->currentIndex() > 0) {
        selectedCategory();
        if (functionIndex >= 0 && functionIndex < functions_list->count()) {
            functions_list->setCurrentIndex(functionIndex);
        }
        selectedFunction();
        if (optionIndex >= 0 && optionIndex < option_list->count()) {
            option_list->setCurrentIndex(optionIndex);
        }
        selectedOption();
    } else {
        reset();
    }

    text_edit->setText(inputValue);
    text_browser->setPlainText(outputValue);
}

void Command::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        event->accept();
    } else {
        QWidget::changeEvent(event);
    }
}

void Command::selectedCategory()
{
    const int selectedIndex = category_list->currentIndex();

    functions_list->clear();

    const QList<CommandFunction> commandList =
        CommandService::functions(static_cast<CommandCategory>(selectedIndex));
    if (commandList.isEmpty()) {
        reset();
        return;
    }

    QStringList commandNames;
    for (const CommandFunction &cmd : commandList) {
        commandNames.append(cmd.getName() + " " + cmd.getDescription());
    }
    functions_list->addItems(commandNames);
    functions_label->setVisible(true);
    functions_list->setVisible(true);
    functions_list->setEnabled(true);
}

void Command::selectedFunction()
{
    text_edit->clear();
    option_list->clear();
    option_list->setVisible(false);
    option_list->setEnabled(false);

    const int selectedFunctionsIndex = functions_list->currentIndex();
    const QList<CommandFunction> commandList =
        CommandService::functions(static_cast<CommandCategory>(category_list->currentIndex()));

    if (selectedFunctionsIndex < 0 || selectedFunctionsIndex >= commandList.size()) {
        return;
    }
    QList<CommandOption> const optionList = commandList[selectedFunctionsIndex].getOptions();

    if (static_cast<int>(!optionList.empty()) != 0) {
        text_edit->clear();
        text_label->setVisible(false);
        text_edit->setVisible(false);
        QStringList optionNames;

        for (const CommandOption &opt : optionList) {
            optionNames.append(opt.getName());
        }
        option_list->addItems(optionNames);
        option_label->setVisible(true);
        option_list->setVisible(true);
        option_list->setEnabled(true);
    }
}

void Command::selectedOption()
{
    const int selectedFunctionsIndex = functions_list->currentIndex();
    const int selectedOptionIndex = option_list->currentIndex();
    const QList<CommandFunction> commandList =
        CommandService::functions(static_cast<CommandCategory>(category_list->currentIndex()));
    if (selectedFunctionsIndex < 0 || selectedFunctionsIndex >= commandList.size()) {
        return;
    }

    QList<CommandOption> optionList = commandList[selectedFunctionsIndex].getOptions();
    if (selectedOptionIndex < 0 || selectedOptionIndex >= optionList.size()) {
        return;
    }

    const CommandOption &option = optionList[selectedOptionIndex];

    if (option.isRequired()) {
        text_label->setText(option.getTitle());
        text_label->setVisible(true);
        text_edit->setVisible(true);
    } else {
        text_edit->clear();
        text_label->setVisible(false);
        text_edit->setVisible(false);
    }
}

void Command::reset()
{
    category_list->setCurrentIndex(0);

    functions_list->clear();
    option_list->clear();

    text_edit->clear();

    text_label->setVisible(false);
    text_edit->setVisible(false);

    functions_label->setVisible(false);
    functions_list->setVisible(false);
    functions_list->setEnabled(false);

    option_label->setVisible(false);
    option_list->setVisible(false);
    option_list->setEnabled(false);
}

void Command::clear()
{
    text_browser->clear();
}

void showErrorAlert()
{
    QMessageBox::critical(nullptr, Command::tr("Error"),
                          Command::tr("This value is invalid. Please check and try again."));
}

void Command::generate()
{
    const CommandGenerationResult result = CommandService::generate(
        static_cast<CommandCategory>(category_list->currentIndex()), functions_list->currentIndex(),
        option_list->currentIndex(), text_edit->text());
    if (result.error == CommandGenerationError::InvalidValue) {
        showErrorAlert();
    } else if (result.error == CommandGenerationError::None) {
        text_browser->setPlainText(result.command);
    }
}

void Command::copy()
{
    const QString text = text_browser->toPlainText();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
