#include "qr_code_generation_gui.h"
#include "ui_qr_code_generation_gui.h"
#include <QMessageBox>
#include <QTextEdit>
#include <QFormLayout>
#include <QLabel>
#include <QClipboard>
#include <QFileDialog>
#include <QApplication>
#include <QLineEdit>
#include <QCheckBox>

QRCodeGenerationGUI::QRCodeGenerationGUI(QWidget *parent)
    : GuiTool(parent)
    , ui(new Ui::QRCodeGenerationGUI)
    , currentType(Text)
{
    ui->setupUi(this);
    
    parameterStack = ui->parameterStackedWidget;
    
    initializeCategories();
    setupParameterWidgets();
    
    connect(ui->categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QRCodeGenerationGUI::onCategoryChanged);
    connect(ui->generateButton, &QPushButton::clicked, this, &QRCodeGenerationGUI::onGenerateClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &QRCodeGenerationGUI::onClearClicked);
    connect(ui->copyButton, &QPushButton::clicked, this, &QRCodeGenerationGUI::onCopyClicked);
    connect(ui->saveButton, &QPushButton::clicked, this, &QRCodeGenerationGUI::onSaveClicked);
    
    // Set initial category
    onCategoryChanged(0);
}

QRCodeGenerationGUI::~QRCodeGenerationGUI()
{
    delete ui;
}

void QRCodeGenerationGUI::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void QRCodeGenerationGUI::initializeCategories()
{
    // Categories are already added in the UI file
    // This function can be used for runtime initialization if needed
}

void QRCodeGenerationGUI::setupParameterWidgets()
{
    // Create and add parameter widget for text type
    parameterStack->addWidget(createTextWidget());      // Index 1
}

QWidget* QRCodeGenerationGUI::createTextWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QTextEdit* textEdit = new QTextEdit();
    textEdit->setPlaceholderText(tr("Enter text..."));
    layout->addRow(tr("Text:"), textEdit);
    
    parameterWidgets["text_content"] = textEdit;
    connect(textEdit, &QTextEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}


void QRCodeGenerationGUI::onCategoryChanged(int index)
{
    // Only Text type is supported now
    currentType = Text;
    
    // Switch to the text parameter widget (index 1)
    parameterStack->setCurrentIndex(1);
    
    clearAllParameters();
    updateGenerateButtonState();
}

void QRCodeGenerationGUI::clearAllParameters()
{
    // Clear all input fields
    for (auto widget : parameterWidgets.values()) {
        if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->clear();
        } else if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
            textEdit->clear();
        } else if (QComboBox* combo = qobject_cast<QComboBox*>(widget)) {
            combo->setCurrentIndex(0);
        } else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
            checkBox->setChecked(false);
        }
    }
    
    ui->contentPreviewEdit->clear();
}

QString QRCodeGenerationGUI::generateQRCodeContent()
{
    QString content;
    
    // Only handle Text type now
    if (QTextEdit* edit = qobject_cast<QTextEdit*>(parameterWidgets["text_content"])) {
        content = edit->toPlainText();
    }
    
    return content;
}

void QRCodeGenerationGUI::updateGenerateButtonState()
{
    bool enabled = false;
    
    // Only handle Text type now
    if (QTextEdit* edit = qobject_cast<QTextEdit*>(parameterWidgets["text_content"])) {
        enabled = !edit->toPlainText().isEmpty();
    }
    
    ui->generateButton->setEnabled(enabled);
}

void QRCodeGenerationGUI::onGenerateClicked()
{
    QString content = generateQRCodeContent();
    
    if (content.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please fill in required fields"));
        return;
    }
    
    // Display content preview
    ui->contentPreviewEdit->setPlainText(content);
    
    // TODO: Actually generate QR code image
    ui->qrCodeLabel->setText(tr("QR Code Generated\n(Implementation pending)"));
    
    // Enable output buttons
    ui->copyButton->setEnabled(true);
    ui->saveButton->setEnabled(true);
}

void QRCodeGenerationGUI::onClearClicked()
{
    clearAllParameters();
    ui->qrCodeLabel->setText(tr("QR Code will appear here"));
    ui->copyButton->setEnabled(false);
    ui->saveButton->setEnabled(false);
}

void QRCodeGenerationGUI::onCopyClicked()
{
    // TODO: Copy QR code image to clipboard
    QApplication::clipboard()->setText(ui->contentPreviewEdit->toPlainText());
    QMessageBox::information(this, tr("Copied"), tr("QR code content copied to clipboard"));
}

void QRCodeGenerationGUI::onSaveClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save QR Code"), "", tr("PNG Files (*.png)"));
    if (!fileName.isEmpty()) {
        // TODO: Save QR code image
        QMessageBox::information(this, tr("Saved"), tr("QR code saved (implementation pending)"));
    }
}