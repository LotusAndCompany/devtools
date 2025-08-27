#include "qr_code_generation_gui.h"
#include "ui_qr_code_generation_gui.h"
#include <QMessageBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDateTimeEdit>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QClipboard>
#include <QFileDialog>
#include <QApplication>

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
    // Create and add parameter widgets for each type
    parameterStack->addWidget(createTextWidget());      // Index 1
    parameterStack->addWidget(createURLWidget());       // Index 2
    parameterStack->addWidget(createEmailWidget());     // Index 3
    parameterStack->addWidget(createPhoneWidget());     // Index 4
    parameterStack->addWidget(createSMSWidget());       // Index 5
    parameterStack->addWidget(createWiFiWidget());      // Index 6
    parameterStack->addWidget(createContactWidget());   // Index 7
    parameterStack->addWidget(createCalendarWidget());  // Index 8
    parameterStack->addWidget(createGeoWidget());       // Index 9
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

QWidget* QRCodeGenerationGUI::createURLWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLineEdit* urlEdit = new QLineEdit();
    urlEdit->setPlaceholderText(tr("https://example.com"));
    layout->addRow(tr("URL:"), urlEdit);
    
    parameterWidgets["url_address"] = urlEdit;
    connect(urlEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createEmailWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLineEdit* emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText(tr("user@example.com"));
    layout->addRow(tr("Email:"), emailEdit);
    
    parameterWidgets["email_address"] = emailEdit;
    connect(emailEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createPhoneWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLineEdit* phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(tr("+1234567890"));
    layout->addRow(tr("Phone Number:"), phoneEdit);
    
    parameterWidgets["phone_number"] = phoneEdit;
    connect(phoneEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createSMSWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLineEdit* phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(tr("+1234567890"));
    layout->addRow(tr("Phone Number:"), phoneEdit);
    
    QTextEdit* messageEdit = new QTextEdit();
    messageEdit->setPlaceholderText(tr("Message text (optional)"));
    messageEdit->setMaximumHeight(100);
    layout->addRow(tr("Message:"), messageEdit);
    
    parameterWidgets["sms_number"] = phoneEdit;
    parameterWidgets["sms_message"] = messageEdit;
    connect(phoneEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createWiFiWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLineEdit* ssidEdit = new QLineEdit();
    ssidEdit->setPlaceholderText(tr("Network name"));
    layout->addRow(tr("SSID:"), ssidEdit);
    
    QLineEdit* passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText(tr("Password (optional)"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addRow(tr("Password:"), passwordEdit);
    
    QComboBox* securityCombo = new QComboBox();
    securityCombo->addItems({tr("WPA/WPA2"), tr("WEP"), tr("No password")});
    layout->addRow(tr("Security:"), securityCombo);
    
    QCheckBox* hiddenCheck = new QCheckBox(tr("Hidden network"));
    layout->addRow("", hiddenCheck);
    
    parameterWidgets["wifi_ssid"] = ssidEdit;
    parameterWidgets["wifi_password"] = passwordEdit;
    parameterWidgets["wifi_security"] = securityCombo;
    parameterWidgets["wifi_hidden"] = hiddenCheck;
    connect(ssidEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createContactWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLineEdit* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText(tr("Full name"));
    layout->addRow(tr("Name:"), nameEdit);
    
    QLineEdit* companyEdit = new QLineEdit();
    companyEdit->setPlaceholderText(tr("Company name (optional)"));
    layout->addRow(tr("Company:"), companyEdit);
    
    QLineEdit* phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(tr("Phone number (optional)"));
    layout->addRow(tr("Phone:"), phoneEdit);
    
    QLineEdit* emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText(tr("Email (optional)"));
    layout->addRow(tr("Email:"), emailEdit);
    
    QLineEdit* addressEdit = new QLineEdit();
    addressEdit->setPlaceholderText(tr("Address (optional)"));
    layout->addRow(tr("Address:"), addressEdit);
    
    QLineEdit* websiteEdit = new QLineEdit();
    websiteEdit->setPlaceholderText(tr("Website (optional)"));
    layout->addRow(tr("Website:"), websiteEdit);
    
    QTextEdit* memoEdit = new QTextEdit();
    memoEdit->setPlaceholderText(tr("Memo (optional)"));
    memoEdit->setMaximumHeight(60);
    layout->addRow(tr("Memo:"), memoEdit);
    
    parameterWidgets["contact_name"] = nameEdit;
    parameterWidgets["contact_company"] = companyEdit;
    parameterWidgets["contact_phone"] = phoneEdit;
    parameterWidgets["contact_email"] = emailEdit;
    parameterWidgets["contact_address"] = addressEdit;
    parameterWidgets["contact_website"] = websiteEdit;
    parameterWidgets["contact_memo"] = memoEdit;
    connect(nameEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createCalendarWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLineEdit* summaryEdit = new QLineEdit();
    summaryEdit->setPlaceholderText(tr("Event name"));
    layout->addRow(tr("Event:"), summaryEdit);
    
    QDateTimeEdit* startEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    startEdit->setCalendarPopup(true);
    layout->addRow(tr("Start:"), startEdit);
    
    QDateTimeEdit* endEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(3600));
    endEdit->setCalendarPopup(true);
    layout->addRow(tr("End:"), endEdit);
    
    QLineEdit* locationEdit = new QLineEdit();
    locationEdit->setPlaceholderText(tr("Location (optional)"));
    layout->addRow(tr("Location:"), locationEdit);
    
    QTextEdit* descriptionEdit = new QTextEdit();
    descriptionEdit->setPlaceholderText(tr("Description (optional)"));
    descriptionEdit->setMaximumHeight(80);
    layout->addRow(tr("Description:"), descriptionEdit);
    
    parameterWidgets["cal_summary"] = summaryEdit;
    parameterWidgets["cal_start"] = startEdit;
    parameterWidgets["cal_end"] = endEdit;
    parameterWidgets["cal_location"] = locationEdit;
    parameterWidgets["cal_description"] = descriptionEdit;
    connect(summaryEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createGeoWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLineEdit* latEdit = new QLineEdit();
    latEdit->setPlaceholderText(tr("e.g., 35.6762"));
    layout->addRow(tr("Latitude:"), latEdit);
    
    QLineEdit* lngEdit = new QLineEdit();
    lngEdit->setPlaceholderText(tr("e.g., 139.6503"));
    layout->addRow(tr("Longitude:"), lngEdit);
    
    QLineEdit* queryEdit = new QLineEdit();
    queryEdit->setPlaceholderText(tr("Search query (optional)"));
    layout->addRow(tr("Query:"), queryEdit);
    
    parameterWidgets["geo_lat"] = latEdit;
    parameterWidgets["geo_lng"] = lngEdit;
    parameterWidgets["geo_query"] = queryEdit;
    connect(latEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(lngEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

void QRCodeGenerationGUI::onCategoryChanged(int index)
{
    currentType = static_cast<QRCodeType>(index);
    
    // Switch to the corresponding parameter widget
    // Index 0 is the empty page, so we add 1
    parameterStack->setCurrentIndex(index + 1);
    
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
    
    switch (currentType) {
    case Text:
        if (QTextEdit* edit = qobject_cast<QTextEdit*>(parameterWidgets["text_content"])) {
            content = edit->toPlainText();
        }
        break;
        
    case URL:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["url_address"])) {
            content = edit->text();
        }
        break;
        
    case Email:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["email_address"])) {
            content = QString("mailto:%1").arg(edit->text());
        }
        break;
        
    case Phone:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["phone_number"])) {
            content = QString("tel:%1").arg(edit->text());
        }
        break;
        
    case SMS:
        if (QLineEdit* numberEdit = qobject_cast<QLineEdit*>(parameterWidgets["sms_number"])) {
            QString number = numberEdit->text();
            QString message;
            if (QTextEdit* msgEdit = qobject_cast<QTextEdit*>(parameterWidgets["sms_message"])) {
                message = msgEdit->toPlainText();
            }
            content = QString("sms:%1%2").arg(number, message.isEmpty() ? "" : "?" + message);
        }
        break;
        
    case WiFi:
        if (QLineEdit* ssidEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_ssid"])) {
            QString ssid = ssidEdit->text();
            QString password;
            QString security = "WPA";
            bool hidden = false;
            
            if (QLineEdit* pwdEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_password"])) {
                password = pwdEdit->text();
            }
            if (QComboBox* secCombo = qobject_cast<QComboBox*>(parameterWidgets["wifi_security"])) {
                int idx = secCombo->currentIndex();
                security = (idx == 0) ? "WPA" : (idx == 1) ? "WEP" : "nopass";
            }
            if (QCheckBox* hiddenCheck = qobject_cast<QCheckBox*>(parameterWidgets["wifi_hidden"])) {
                hidden = hiddenCheck->isChecked();
            }
            
            content = QString("WIFI:T:%1;S:%2;P:%3;H:%4;;")
                .arg(security)
                .arg(ssid)
                .arg(password)
                .arg(hidden ? "true" : "false");
        }
        break;
        
    case Contact:
        // Generate vCard format
        content = "BEGIN:VCARD\nVERSION:3.0\n";
        if (QLineEdit* nameEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_name"])) {
            content += QString("FN:%1\n").arg(nameEdit->text());
        }
        if (QLineEdit* companyEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_company"])) {
            if (!companyEdit->text().isEmpty())
                content += QString("ORG:%1\n").arg(companyEdit->text());
        }
        if (QLineEdit* phoneEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_phone"])) {
            if (!phoneEdit->text().isEmpty())
                content += QString("TEL:%1\n").arg(phoneEdit->text());
        }
        if (QLineEdit* emailEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_email"])) {
            if (!emailEdit->text().isEmpty())
                content += QString("EMAIL:%1\n").arg(emailEdit->text());
        }
        if (QLineEdit* addressEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_address"])) {
            if (!addressEdit->text().isEmpty())
                content += QString("ADR:%1\n").arg(addressEdit->text());
        }
        if (QLineEdit* websiteEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_website"])) {
            if (!websiteEdit->text().isEmpty())
                content += QString("URL:%1\n").arg(websiteEdit->text());
        }
        if (QTextEdit* memoEdit = qobject_cast<QTextEdit*>(parameterWidgets["contact_memo"])) {
            if (!memoEdit->toPlainText().isEmpty())
                content += QString("NOTE:%1\n").arg(memoEdit->toPlainText());
        }
        content += "END:VCARD";
        break;
        
    case CalendarEvent:
        // Generate iCalendar format
        content = "BEGIN:VEVENT\n";
        if (QLineEdit* summaryEdit = qobject_cast<QLineEdit*>(parameterWidgets["cal_summary"])) {
            content += QString("SUMMARY:%1\n").arg(summaryEdit->text());
        }
        if (QDateTimeEdit* startEdit = qobject_cast<QDateTimeEdit*>(parameterWidgets["cal_start"])) {
            content += QString("DTSTART:%1\n").arg(startEdit->dateTime().toString("yyyyMMddTHHmmss"));
        }
        if (QDateTimeEdit* endEdit = qobject_cast<QDateTimeEdit*>(parameterWidgets["cal_end"])) {
            content += QString("DTEND:%1\n").arg(endEdit->dateTime().toString("yyyyMMddTHHmmss"));
        }
        if (QLineEdit* locationEdit = qobject_cast<QLineEdit*>(parameterWidgets["cal_location"])) {
            if (!locationEdit->text().isEmpty())
                content += QString("LOCATION:%1\n").arg(locationEdit->text());
        }
        if (QTextEdit* descEdit = qobject_cast<QTextEdit*>(parameterWidgets["cal_description"])) {
            if (!descEdit->toPlainText().isEmpty())
                content += QString("DESCRIPTION:%1\n").arg(descEdit->toPlainText());
        }
        content += "END:VEVENT";
        break;
        
    case GeoLocation:
        if (QLineEdit* latEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lat"])) {
            if (QLineEdit* lngEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lng"])) {
                QString lat = latEdit->text();
                QString lng = lngEdit->text();
                QString query;
                if (QLineEdit* queryEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_query"])) {
                    query = queryEdit->text();
                }
                content = QString("geo:%1,%2%3").arg(lat, lng, query.isEmpty() ? "" : "?q=" + query);
            }
        }
        break;
    }
    
    return content;
}

void QRCodeGenerationGUI::updateGenerateButtonState()
{
    bool enabled = false;
    
    switch (currentType) {
    case Text:
        if (QTextEdit* edit = qobject_cast<QTextEdit*>(parameterWidgets["text_content"])) {
            enabled = !edit->toPlainText().isEmpty();
        }
        break;
        
    case URL:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["url_address"])) {
            enabled = !edit->text().isEmpty();
        }
        break;
        
    case Email:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["email_address"])) {
            enabled = !edit->text().isEmpty();
        }
        break;
        
    case Phone:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["phone_number"])) {
            enabled = !edit->text().isEmpty();
        }
        break;
        
    case SMS:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["sms_number"])) {
            enabled = !edit->text().isEmpty();
        }
        break;
        
    case WiFi:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_ssid"])) {
            enabled = !edit->text().isEmpty();
        }
        break;
        
    case Contact:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["contact_name"])) {
            enabled = !edit->text().isEmpty();
        }
        break;
        
    case CalendarEvent:
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["cal_summary"])) {
            enabled = !edit->text().isEmpty();
        }
        break;
        
    case GeoLocation:
        if (QLineEdit* latEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lat"])) {
            if (QLineEdit* lngEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lng"])) {
                enabled = !latEdit->text().isEmpty() && !lngEdit->text().isEmpty();
            }
        }
        break;
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