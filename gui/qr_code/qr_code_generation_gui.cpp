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
#include <QImage>
#include "core/qr_tool/qrcodegen.hpp"
#include "core/qr_tool/content_generator.h"

using qrcodegen::QrCode;
using qrcodegen::QrSegment;

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
    QVariantMap params;
    QString type;

    switch (currentType) {
    case Text: {
        if (QTextEdit* edit = qobject_cast<QTextEdit*>(parameterWidgets["text_content"])) {
            params["text"] = edit->toPlainText();
        }
        type = "text";
        break;
    }

    // ↓ UI未構築なので一旦コメントアウト
    // TODO: UI構築したものから都度コメントアウト解除

    // case Url: {
    //     if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["url_content"])) {
    //         params["url"] = edit->text();
    //     }
    //     type = "url";
    //     break;
    // }

    // case Email: {
    //     if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["email_address"])) {
    //         params["email"] = edit->text();
    //     }
    //     type = "email";
    //     break;
    // }

    // case Phone: {
    //     if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["phone_number"])) {
    //         params["number"] = edit->text();
    //     }
    //     type = "phone";
    //     break;
    // }

    // case Sms: {
    //     if (QLineEdit* numEdit = qobject_cast<QLineEdit*>(parameterWidgets["sms_number"])) {
    //         params["number"] = numEdit->text();
    //     }
    //     if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(parameterWidgets["sms_text"])) {
    //         params["text"] = textEdit->toPlainText();
    //     }
    //     type = "sms";
    //     break;
    // }

    // case Geo: {
    //     if (QLineEdit* latEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lat"])) {
    //         params["lat"] = latEdit->text().toDouble();
    //     }
    //     if (QLineEdit* lngEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lng"])) {
    //         params["lng"] = lngEdit->text().toDouble();
    //     }
    //     type = "geo";
    //     break;
    // }

    // case Calendar: {
    //     if (QLineEdit* summaryEdit = qobject_cast<QLineEdit*>(parameterWidgets["cal_summary"])) {
    //         params["summary"] = summaryEdit->text();
    //     }
    //     if (QLineEdit* startEdit = qobject_cast<QLineEdit*>(parameterWidgets["cal_start"])) {
    //         params["start"] = startEdit->text();
    //     }
    //     if (QLineEdit* endEdit = qobject_cast<QLineEdit*>(parameterWidgets["cal_end"])) {
    //         params["end"] = endEdit->text();
    //     }
    //     type = "calendar";
    //     break;
    // }

    // case Contact: {
    //     if (QLineEdit* nameEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_name"])) {
    //         params["name"] = nameEdit->text();
    //     }
    //     if (QLineEdit* phoneEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_phone"])) {
    //         params["phone"] = phoneEdit->text();
    //     }
    //     if (QLineEdit* emailEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_email"])) {
    //         params["email"] = emailEdit->text();
    //     }
    //     type = "contact";
    //     break;
    // }

    // case Wifi: {
    //     if (QLineEdit* ssidEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_ssid"])) {
    //         params["ssid"] = ssidEdit->text();
    //     }
    //     if (QLineEdit* passEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_password"])) {
    //         params["password"] = passEdit->text();
    //     }
    //     if (QComboBox* combo = qobject_cast<QComboBox*>(parameterWidgets["wifi_type"])) {
    //         params["type"] = combo->currentText();  // WPA/WEP/nopass
    //     }
    //     if (QCheckBox* hiddenCheck = qobject_cast<QCheckBox*>(parameterWidgets["wifi_hidden"])) {
    //         params["hidden"] = hiddenCheck->isChecked();
    //     }
    //     type = "wifi";
    //     break;
    // }

    default:
        type = "text";
        params["text"] = "default";
        break;
    }

    ContentGenerator generator;
    return generator.generate(type, params);
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
    
    // QRコード生成
    const QrCode qr = QrCode::encodeText(content.toUtf8().constData(), QrCode::Ecc::LOW);

    int scale = 8; // 自由に設定させるかは要検討

    int size = qr.getSize();
    QImage image(size * scale, size * scale, QImage::Format_RGB32);
    image.fill(Qt::white);

    // QRコード描画
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (qr.getModule(x, y)) {
                for (int dy = 0; dy < scale; dy++) {
                    for (int dx = 0; dx < scale; dx++) {
                        image.setPixel(x * scale + dx, y * scale + dy, qRgb(0, 0, 0));
                    }
                }
            }
        }
    }

    ui->qrCodeLabel->setPixmap(QPixmap::fromImage(image));
    // 枠いっぱいにするかどうか
    //ui->qrCodeLabel->setScaledContents(true);  // サイズに合わせて拡大縮小
    ui->qrCodeLabel->show();
    
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
