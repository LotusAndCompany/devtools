#include "qr_code_generation_gui.h"
#include "ui_qr_code_generation_gui.h"
#include <QMessageBox>
#include <QTextEdit>
#include <QLabel>
#include <QClipboard>
#include <QFileDialog>
#include <QApplication>
#include <QVariantMap>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
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
    // Create and add parameter widgets for all types
    parameterStack->addWidget(createTextWidget());      // Index 1
    parameterStack->addWidget(createUrlWidget());       // Index 2
    parameterStack->addWidget(createEmailWidget());     // Index 3
    parameterStack->addWidget(createPhoneWidget());     // Index 4
    parameterStack->addWidget(createSmsWidget());       // Index 5
    parameterStack->addWidget(createWifiWidget());      // Index 6
    parameterStack->addWidget(createContactWidget());   // Index 7
    parameterStack->addWidget(createCalendarWidget());  // Index 8
    parameterStack->addWidget(createGeoWidget());       // Index 9
}

QWidget* QRCodeGenerationGUI::createTextWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    
    QTextEdit* textEdit = new QTextEdit();
    textEdit->setPlaceholderText(tr("Enter text..."));
    textEdit->setMinimumHeight(100);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addRow(tr("Text:"), textEdit);
    
    // エラーラベル作成
    QLabel* errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: red; font-size: 12px;");
    errorLabel->setVisible(false);
    errorLabels["text_content_error"] = errorLabel;
    layout->addRow("", errorLabel);
    
    parameterWidgets["text_content"] = textEdit;
    connect(textEdit, &QTextEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createUrlWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    
    QLineEdit* urlEdit = new QLineEdit();
    urlEdit->setPlaceholderText(tr("Enter URL..."));
    urlEdit->setMinimumWidth(200);
    urlEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("URL:"), urlEdit);
    
    // エラーラベル作成
    QLabel* errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: red; font-size: 12px;");
    errorLabel->setVisible(false);
    errorLabels["url_content_error"] = errorLabel;
    layout->addRow("", errorLabel);
    
    parameterWidgets["url_content"] = urlEdit;
    connect(urlEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createEmailWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    
    QLineEdit* emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText(tr("Enter email address..."));
    emailEdit->setMinimumWidth(200);
    emailEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Email:"), emailEdit);
    
    // エラーラベル作成
    QLabel* errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: red; font-size: 12px;");
    errorLabel->setVisible(false);
    errorLabels["email_address_error"] = errorLabel;
    layout->addRow("", errorLabel);
    
    parameterWidgets["email_address"] = emailEdit;
    connect(emailEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createPhoneWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    
    QLineEdit* phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(tr("Enter phone number..."));
    phoneEdit->setMinimumWidth(200);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Phone:"), phoneEdit);
    
    // エラーラベル作成
    QLabel* errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: red; font-size: 12px;");
    errorLabel->setVisible(false);
    errorLabels["phone_number_error"] = errorLabel;
    layout->addRow("", errorLabel);
    
    parameterWidgets["phone_number"] = phoneEdit;
    connect(phoneEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createSmsWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    
    QLineEdit* phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(tr("Enter phone number..."));
    phoneEdit->setMinimumWidth(200);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Phone:"), phoneEdit);
    
    // 電話番号エラーラベル作成
    QLabel* phoneErrorLabel = new QLabel();
    phoneErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    phoneErrorLabel->setVisible(false);
    errorLabels["sms_phone_error"] = phoneErrorLabel;
    layout->addRow("", phoneErrorLabel);
    
    QTextEdit* messageEdit = new QTextEdit();
    messageEdit->setPlaceholderText(tr("Enter message..."));
    messageEdit->setMinimumHeight(80);
    messageEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addRow(tr("Message:"), messageEdit);
    
    // メッセージエラーラベル作成
    QLabel* messageErrorLabel = new QLabel();
    messageErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    messageErrorLabel->setVisible(false);
    errorLabels["sms_message_error"] = messageErrorLabel;
    layout->addRow("", messageErrorLabel);
    
    parameterWidgets["sms_phone"] = phoneEdit;
    parameterWidgets["sms_message"] = messageEdit;
    connect(phoneEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(messageEdit, &QTextEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createWifiWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    
    QLineEdit* ssidEdit = new QLineEdit();
    ssidEdit->setPlaceholderText(tr("Enter SSID..."));
    ssidEdit->setMinimumWidth(200);
    ssidEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("SSID:"), ssidEdit);
    
    // SSIDエラーラベル作成
    QLabel* ssidErrorLabel = new QLabel();
    ssidErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    ssidErrorLabel->setVisible(false);
    errorLabels["wifi_ssid_error"] = ssidErrorLabel;
    layout->addRow("", ssidErrorLabel);
    
    QLineEdit* passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText(tr("Enter password..."));
    passwordEdit->setMinimumWidth(200);
    passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Password:"), passwordEdit);
    
    // パスワードエラーラベル作成
    QLabel* passwordErrorLabel = new QLabel();
    passwordErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    passwordErrorLabel->setVisible(false);
    errorLabels["wifi_password_error"] = passwordErrorLabel;
    layout->addRow("", passwordErrorLabel);
    
    QComboBox* securityCombo = new QComboBox();
    securityCombo->addItem("WPA");
    securityCombo->addItem("WEP");
    securityCombo->addItem("nopass");
    layout->addRow(tr("Security:"), securityCombo);
    
    QCheckBox* hiddenCheck = new QCheckBox();
    layout->addRow(tr("Hidden Network"), hiddenCheck);
    
    parameterWidgets["wifi_ssid"] = ssidEdit;
    parameterWidgets["wifi_password"] = passwordEdit;
    parameterWidgets["wifi_security"] = securityCombo;
    parameterWidgets["wifi_hidden"] = hiddenCheck;
    connect(ssidEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(passwordEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(securityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createContactWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    
    QLineEdit* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText(tr("Enter name..."));
    layout->addRow(tr("Name:"), nameEdit);
    
    // 名前エラーラベル作成
    QLabel* nameErrorLabel = new QLabel();
    nameErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    nameErrorLabel->setVisible(false);
    errorLabels["contact_name_error"] = nameErrorLabel;
    layout->addRow("", nameErrorLabel);
    
    QLineEdit* phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(tr("Enter phone..."));
    phoneEdit->setMinimumWidth(200);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Phone:"), phoneEdit);
    
    // 電話エラーラベル作成
    QLabel* phoneErrorLabel = new QLabel();
    phoneErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    phoneErrorLabel->setVisible(false);
    errorLabels["contact_phone_error"] = phoneErrorLabel;
    layout->addRow("", phoneErrorLabel);
    
    QLineEdit* emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText(tr("Enter email..."));
    emailEdit->setMinimumWidth(200);
    emailEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Email:"), emailEdit);
    
    // メールエラーラベル作成
    QLabel* emailErrorLabel = new QLabel();
    emailErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    emailErrorLabel->setVisible(false);
    errorLabels["contact_email_error"] = emailErrorLabel;
    layout->addRow("", emailErrorLabel);
    
    parameterWidgets["contact_name"] = nameEdit;
    parameterWidgets["contact_phone"] = phoneEdit;
    parameterWidgets["contact_email"] = emailEdit;
    connect(nameEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(phoneEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(emailEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createCalendarWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    
    QLineEdit* summaryEdit = new QLineEdit();
    summaryEdit->setPlaceholderText(tr("Enter event title..."));
    summaryEdit->setMinimumWidth(200);
    summaryEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Event:"), summaryEdit);
    
    // イベントタイトルエラーラベル作成
    QLabel* summaryErrorLabel = new QLabel();
    summaryErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    summaryErrorLabel->setVisible(false);
    errorLabels["cal_summary_error"] = summaryErrorLabel;
    layout->addRow("", summaryErrorLabel);
    
    QDateTimeEdit* startEdit = new QDateTimeEdit();
    startEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    startEdit->setDateTime(QDateTime::currentDateTime());
    startEdit->setMinimumWidth(200);
    startEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    startEdit->setCalendarPopup(true);
    layout->addRow(tr("Start:"), startEdit);
    
    // 開始時刻エラーラベル作成
    QLabel* startErrorLabel = new QLabel();
    startErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    startErrorLabel->setVisible(false);
    errorLabels["cal_start_error"] = startErrorLabel;
    layout->addRow("", startErrorLabel);
    
    QDateTimeEdit* endEdit = new QDateTimeEdit();
    endEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    endEdit->setDateTime(QDateTime::currentDateTime().addSecs(3600));
    endEdit->setMinimumWidth(200);
    endEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    endEdit->setCalendarPopup(true);
    layout->addRow(tr("End:"), endEdit);
    
    // 終了時刻エラーラベル作成
    QLabel* endErrorLabel = new QLabel();
    endErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    endErrorLabel->setVisible(false);
    errorLabels["cal_end_error"] = endErrorLabel;
    layout->addRow("", endErrorLabel);
    
    parameterWidgets["cal_summary"] = summaryEdit;
    parameterWidgets["cal_start"] = startEdit;
    parameterWidgets["cal_end"] = endEdit;
    connect(summaryEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(startEdit, &QDateTimeEdit::dateTimeChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(endEdit, &QDateTimeEdit::dateTimeChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}

QWidget* QRCodeGenerationGUI::createGeoWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    
    QLineEdit* latEdit = new QLineEdit();
    latEdit->setPlaceholderText(tr("Enter latitude..."));
    latEdit->setMinimumWidth(200);
    latEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Latitude:"), latEdit);
    
    // 緯度エラーラベル作成
    QLabel* latErrorLabel = new QLabel();
    latErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    latErrorLabel->setVisible(false);
    errorLabels["geo_lat_error"] = latErrorLabel;
    layout->addRow("", latErrorLabel);
    
    QLineEdit* lngEdit = new QLineEdit();
    lngEdit->setPlaceholderText(tr("Enter longitude..."));
    lngEdit->setMinimumWidth(200);
    lngEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Longitude:"), lngEdit);
    
    // 経度エラーラベル作成
    QLabel* lngErrorLabel = new QLabel();
    lngErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    lngErrorLabel->setVisible(false);
    errorLabels["geo_lng_error"] = lngErrorLabel;
    layout->addRow("", lngErrorLabel);
    
    parameterWidgets["geo_lat"] = latEdit;
    parameterWidgets["geo_lng"] = lngEdit;
    connect(latEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(lngEdit, &QLineEdit::textChanged, this, &QRCodeGenerationGUI::updateGenerateButtonState);
    
    return widget;
}


void QRCodeGenerationGUI::onCategoryChanged(int index)
{
    // Map combo box index to QRCodeType and parameterStack index
    switch (index) {
    case 0:  // Text
        currentType = Text;
        parameterStack->setCurrentIndex(1);
        break;
    case 1:  // URL
        currentType = Url;
        parameterStack->setCurrentIndex(2);
        break;
    case 2:  // Email
        currentType = Email;
        parameterStack->setCurrentIndex(3);
        break;
    case 3:  // Phone Number
        currentType = Phone;
        parameterStack->setCurrentIndex(4);
        break;
    case 4:  // SMS
        currentType = Sms;
        parameterStack->setCurrentIndex(5);
        break;
    case 5:  // WiFi
        currentType = Wifi;
        parameterStack->setCurrentIndex(6);
        break;
    case 6:  // Contact
        currentType = Contact;
        parameterStack->setCurrentIndex(7);
        break;
    case 7:  // Calendar
        currentType = Calendar;
        parameterStack->setCurrentIndex(8);
        break;
    case 8:  // Geo Location
        currentType = Geo;
        parameterStack->setCurrentIndex(9);
        break;
    default:
        // Default to Text
        currentType = Text;
        parameterStack->setCurrentIndex(1);
        break;
    }
    
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
    
    // エラーラベルもクリア
    clearValidationErrors();
}

bool QRCodeGenerationGUI::validateCurrentType()
{
    clearValidationErrors();
    bool isValid = true;
    
    switch (currentType) {
    case Text: {
        QTextEdit* edit = qobject_cast<QTextEdit*>(parameterWidgets["text_content"]);
        if (!edit || edit->toPlainText().trimmed().isEmpty()) {
            showValidationError("text_content_error", tr("Text cannot be empty"));
            isValid = false;
        }
        break;
    }
    
    case Url: {
        QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["url_content"]);
        if (!edit || edit->text().trimmed().isEmpty()) {
            showValidationError("url_content_error", tr("URL cannot be empty"));
            isValid = false;
        } else if (!edit->text().startsWith("http://") && !edit->text().startsWith("https://")) {
            showValidationError("url_content_error", tr("URL must start with http:// or https://"));
            isValid = false;
        }
        break;
    }
    
    case Email: {
        QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["email_address"]);
        if (!edit || edit->text().trimmed().isEmpty()) {
            showValidationError("email_address_error", tr("Email cannot be empty"));
            isValid = false;
        } else if (!edit->text().contains("@")) {
            showValidationError("email_address_error", tr("Invalid email format"));
            isValid = false;
        }
        break;
    }
    
    case Phone: {
        QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["phone_number"]);
        if (!edit || edit->text().trimmed().isEmpty()) {
            showValidationError("phone_number_error", tr("Phone number cannot be empty"));
            isValid = false;
        } else if (!isValidPhoneNumber(edit->text())) {
            showValidationError("phone_number_error", tr("Invalid phone number format"));
            isValid = false;
        }
        break;
    }
    
    case Sms: {
        QLineEdit* phoneEdit = qobject_cast<QLineEdit*>(parameterWidgets["sms_phone"]);
        QTextEdit* messageEdit = qobject_cast<QTextEdit*>(parameterWidgets["sms_message"]);
        
        if (!phoneEdit || phoneEdit->text().trimmed().isEmpty()) {
            showValidationError("sms_phone_error", tr("Phone number cannot be empty"));
            isValid = false;
        } else if (!isValidPhoneNumber(phoneEdit->text())) {
            showValidationError("sms_phone_error", tr("Invalid phone number format"));
            isValid = false;
        }
        if (!messageEdit || messageEdit->toPlainText().trimmed().isEmpty()) {
            showValidationError("sms_message_error", tr("Message cannot be empty"));
            isValid = false;
        }
        break;
    }
    
    case Wifi: {
        QLineEdit* ssidEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_ssid"]);
        QLineEdit* passwordEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_password"]);
        QComboBox* securityCombo = qobject_cast<QComboBox*>(parameterWidgets["wifi_security"]);
        
        if (!ssidEdit || ssidEdit->text().trimmed().isEmpty()) {
            showValidationError("wifi_ssid_error", tr("SSID cannot be empty"));
            isValid = false;
        }
        
        // WPAまたはWEPが選択されている場合、パスワードが必須
        if (securityCombo && (securityCombo->currentText() == "WPA" || securityCombo->currentText() == "WEP")) {
            if (!passwordEdit || passwordEdit->text().trimmed().isEmpty()) {
                showValidationError("wifi_password_error", tr("Password is required for WPA/WEP security"));
                isValid = false;
            }
        }
        break;
    }
    
    case Contact: {
        QLineEdit* nameEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_name"]);
        QLineEdit* phoneEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_phone"]);
        QLineEdit* emailEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_email"]);
        
        if (!nameEdit || nameEdit->text().trimmed().isEmpty()) {
            showValidationError("contact_name_error", tr("Name cannot be empty"));
            isValid = false;
        }
        
        // 少なくとも電話番号かメールアドレスのどちらかは必須
        bool hasPhone = phoneEdit && !phoneEdit->text().trimmed().isEmpty();
        bool hasEmail = emailEdit && !emailEdit->text().trimmed().isEmpty();
        
        if (!hasPhone && !hasEmail) {
            showValidationError("contact_phone_error", tr("At least phone or email is required"));
            isValid = false;
        }

        // 電話番号が入力されている場合のフォーマットチェック
        if (hasPhone && !isValidPhoneNumber(phoneEdit->text())) {
            showValidationError("contact_phone_error", tr("Invalid phone number format"));
            isValid = false;
        }
        
        // メールアドレスが入力されている場合のフォーマットチェック
        if (hasEmail && !emailEdit->text().contains("@")) {
            showValidationError("contact_email_error", tr("Invalid email format"));
            isValid = false;
        }
        break;
    }
    
    case Calendar: {
        QLineEdit* summaryEdit = qobject_cast<QLineEdit*>(parameterWidgets["cal_summary"]);
        QDateTimeEdit* startEdit = qobject_cast<QDateTimeEdit*>(parameterWidgets["cal_start"]);
        QDateTimeEdit* endEdit = qobject_cast<QDateTimeEdit*>(parameterWidgets["cal_end"]);
        
        if (!summaryEdit || summaryEdit->text().trimmed().isEmpty()) {
            showValidationError("cal_summary_error", tr("Event title cannot be empty"));
            isValid = false;
        }
        
        if (!startEdit) {
            showValidationError("cal_start_error", tr("Start date/time cannot be empty"));
            isValid = false;
        }
        
        if (!endEdit) {
            showValidationError("cal_end_error", tr("End date/time cannot be empty"));
            isValid = false;
        }
        
        // 開始時間が終了時間より後でないかチェック
        if (startEdit && endEdit && startEdit->dateTime() >= endEdit->dateTime()) {
            showValidationError("cal_end_error", tr("End time must be after start time"));
            isValid = false;
        }
        break;
    }
    
    case Geo: {
        QLineEdit* latEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lat"]);
        QLineEdit* lngEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lng"]);
        
        if (!latEdit || latEdit->text().trimmed().isEmpty()) {
            showValidationError("geo_lat_error", tr("Latitude cannot be empty"));
            isValid = false;
        } else {
            bool ok;
            double lat = latEdit->text().toDouble(&ok);
            if (!ok || lat < -90.0 || lat > 90.0) {
                showValidationError("geo_lat_error", tr("Latitude must be between -90 and 90"));
                isValid = false;
            }
        }
        
        if (!lngEdit || lngEdit->text().trimmed().isEmpty()) {
            showValidationError("geo_lng_error", tr("Longitude cannot be empty"));
            isValid = false;
        } else {
            bool ok;
            double lng = lngEdit->text().toDouble(&ok);
            if (!ok || lng < -180.0 || lng > 180.0) {
                showValidationError("geo_lng_error", tr("Longitude must be between -180 and 180"));
                isValid = false;
            }
        }
        break;
    }
    }
    
    return isValid;
}

void QRCodeGenerationGUI::showValidationError(const QString& fieldKey, const QString& message)
{
    if (QLabel* errorLabel = errorLabels.value(fieldKey)) {
        errorLabel->setText(message);
        errorLabel->setVisible(true);
    }
}

void QRCodeGenerationGUI::clearValidationErrors()
{
    for (auto label : errorLabels.values()) {
        label->setVisible(false);
        label->clear();
    }
}

bool QRCodeGenerationGUI::isValidPhoneNumber(const QString& phone)
{
    if (phone.trimmed().isEmpty()) {
        return false;
    }
    
    QString cleanPhone = phone.trimmed();
    
    // 国際番号形式（+81-xxx-xxxx-xxxx, +81xxxxxxxxxx）
    QRegularExpression internationalPattern(R"(^\+\d{1,3}[-\s]?\d{1,4}[-\s]?\d{4}[-\s]?\d{4}$)");
    if (internationalPattern.match(cleanPhone).hasMatch()) {
        return true;
    }
    
    // 日本の国内番号形式（0xx-xxxx-xxxx, 0xxxxxxxxxx）
    QRegularExpression domesticPattern(R"(^0\d{1,4}[-\s]?\d{2,4}[-\s]?\d{4}$)");
    if (domesticPattern.match(cleanPhone).hasMatch()) {
        return true;
    }
    
    // 数字のみ（10-11桁）
    QRegularExpression digitOnlyPattern(R"(^\d{10,11}$)");
    if (digitOnlyPattern.match(cleanPhone).hasMatch()) {
        return true;
    }
    
    return false;
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

    case Url: {
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["url_content"])) {
            params["url"] = edit->text();
        }
        type = "url";
        break;
    }

    case Email: {
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["email_address"])) {
            params["email"] = edit->text();
        }
        type = "email";
        break;
    }

    case Phone: {
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["phone_number"])) {
            params["number"] = edit->text();
        }
        type = "phone";
        break;
    }

    case Sms: {
        if (QLineEdit* phoneEdit = qobject_cast<QLineEdit*>(parameterWidgets["sms_phone"])) {
            params["number"] = phoneEdit->text();
        }
        if (QTextEdit* messageEdit = qobject_cast<QTextEdit*>(parameterWidgets["sms_message"])) {
            params["text"] = messageEdit->toPlainText();
        }
        type = "sms";
        break;
    }

    case Contact: {
        if (QLineEdit* nameEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_name"])) {
            params["name"] = nameEdit->text();
        }
        if (QLineEdit* phoneEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_phone"])) {
            params["phone"] = phoneEdit->text();
        }
        if (QLineEdit* emailEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_email"])) {
            params["email"] = emailEdit->text();
        }
        type = "contact";
        break;
    }

    case Calendar: {
        if (QLineEdit* summaryEdit = qobject_cast<QLineEdit*>(parameterWidgets["cal_summary"])) {
            params["summary"] = summaryEdit->text();
        }
        if (QDateTimeEdit* startEdit = qobject_cast<QDateTimeEdit*>(parameterWidgets["cal_start"])) {
            params["start"] = startEdit->dateTime().toString("yyyyMMddThhmmss");
        }
        if (QDateTimeEdit* endEdit = qobject_cast<QDateTimeEdit*>(parameterWidgets["cal_end"])) {
            params["end"] = endEdit->dateTime().toString("yyyyMMddThhmmss");
        }
        type = "calendar";
        break;
    }

    case Geo: {
        if (QLineEdit* latEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lat"])) {
            params["lat"] = latEdit->text();
        }
        if (QLineEdit* lngEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lng"])) {
            params["lng"] = lngEdit->text();
        }
        type = "geo";
        break;
    }

    case Wifi: {
        if (QLineEdit* ssidEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_ssid"])) {
            params["ssid"] = ssidEdit->text();
        }
        if (QLineEdit* passEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_password"])) {
            params["password"] = passEdit->text();
        }
        if (QComboBox* securityCombo = qobject_cast<QComboBox*>(parameterWidgets["wifi_security"])) {
            params["type"] = securityCombo->currentText();
        }
        if (QCheckBox* hiddenCheck = qobject_cast<QCheckBox*>(parameterWidgets["wifi_hidden"])) {
            params["hidden"] = hiddenCheck->isChecked() ? "true" : "false";
        }
        type = "wifi";
        break;
    }
    }

    // バリデーションチェック（後で実装される部分なのでシンプルに）
    if (params.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please fill in required fields"));
        return QString();
    }

    ContentGenerator generator;
    return generator.generate(type, params);
}

void QRCodeGenerationGUI::updateGenerateButtonState()
{
    bool enabled = false;
    
    switch (currentType) {
    case Text: {
        if (QTextEdit* edit = qobject_cast<QTextEdit*>(parameterWidgets["text_content"])) {
            enabled = !edit->toPlainText().trimmed().isEmpty();
        }
        break;
    }
    
    case Url: {
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["url_content"])) {
            enabled = !edit->text().trimmed().isEmpty();
        }
        break;
    }
    
    case Email: {
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["email_address"])) {
            enabled = !edit->text().trimmed().isEmpty();
        }
        break;
    }
    
    case Phone: {
        if (QLineEdit* edit = qobject_cast<QLineEdit*>(parameterWidgets["phone_number"])) {
            enabled = !edit->text().trimmed().isEmpty();
        }
        break;
    }
    
    case Sms: {
        QLineEdit* phoneEdit = qobject_cast<QLineEdit*>(parameterWidgets["sms_phone"]);
        QTextEdit* messageEdit = qobject_cast<QTextEdit*>(parameterWidgets["sms_message"]);
        enabled = phoneEdit && messageEdit && 
                  !phoneEdit->text().trimmed().isEmpty() && 
                  !messageEdit->toPlainText().trimmed().isEmpty();
        break;
    }
    
    case Wifi: {
        QLineEdit* ssidEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_ssid"]);
        QComboBox* securityCombo = qobject_cast<QComboBox*>(parameterWidgets["wifi_security"]);
        QLineEdit* passwordEdit = qobject_cast<QLineEdit*>(parameterWidgets["wifi_password"]);
        
        if (ssidEdit && securityCombo) {
            enabled = !ssidEdit->text().trimmed().isEmpty();
            
            // WPA or WEP選択時はパスワード必須
            QString security = securityCombo->currentText();
            if ((security == "WPA" || security == "WEP") && passwordEdit) {
                enabled = enabled && !passwordEdit->text().trimmed().isEmpty();
            }
        }
        break;
    }
    
    case Contact: {
        QLineEdit* nameEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_name"]);
        QLineEdit* phoneEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_phone"]);
        QLineEdit* emailEdit = qobject_cast<QLineEdit*>(parameterWidgets["contact_email"]);
        
        if (nameEdit) {
            bool hasName = !nameEdit->text().trimmed().isEmpty();
            bool hasPhoneOrEmail = false;
            
            if (phoneEdit && !phoneEdit->text().trimmed().isEmpty()) {
                hasPhoneOrEmail = true;
            }
            if (emailEdit && !emailEdit->text().trimmed().isEmpty()) {
                hasPhoneOrEmail = true;
            }
            
            enabled = hasName && hasPhoneOrEmail;
        }
        break;
    }
    
    case Calendar: {
        QLineEdit* summaryEdit = qobject_cast<QLineEdit*>(parameterWidgets["cal_summary"]);
        QDateTimeEdit* startEdit = qobject_cast<QDateTimeEdit*>(parameterWidgets["cal_start"]);
        QDateTimeEdit* endEdit = qobject_cast<QDateTimeEdit*>(parameterWidgets["cal_end"]);
        
        enabled = summaryEdit && startEdit && endEdit &&
                  !summaryEdit->text().trimmed().isEmpty();
        break;
    }
    
    case Geo: {
        QLineEdit* latEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lat"]);
        QLineEdit* lngEdit = qobject_cast<QLineEdit*>(parameterWidgets["geo_lng"]);
        
        enabled = latEdit && lngEdit &&
                  !latEdit->text().trimmed().isEmpty() &&
                  !lngEdit->text().trimmed().isEmpty();
        break;
    }
    }
    
    ui->generateButton->setEnabled(enabled);
}

void QRCodeGenerationGUI::onGenerateClicked()
{
    if (!validateCurrentType()) {
        return; // バリデーションエラーがある場合は処理を中断
    }
    
    QString content = generateQRCodeContent();
    
    if (content.isEmpty()) {
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

    // 生成されたQRコード画像をメンバ変数に保存
    currentQRImage = image;
    
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
    
    // 現在のQRコード画像をクリア
    currentQRImage = QImage();
}

void QRCodeGenerationGUI::onCopyClicked()
{
    // 画像が生成されているかチェック
    if (currentQRImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("QRコードが生成されていません。"));
        return;
    }
    
    // QRコード画像をクリップボードにコピー
    QApplication::clipboard()->setImage(currentQRImage);
    QMessageBox::information(this, tr("Copied"), tr("QRコード画像をクリップボードにコピーしました。"));
}

void QRCodeGenerationGUI::onSaveClicked()
{
    // 画像が生成されているかチェック
    if (currentQRImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("QRコードが生成されていません。"));
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save QR Code"), "", tr("PNG Files (*.png)"));
    if (!fileName.isEmpty()) {
        // ファイル拡張子がない場合は".png"を自動追加
        if (!fileName.toLower().endsWith(".png")) {
            fileName += ".png";
        }
        
        // 実際にファイル保存
        if (currentQRImage.save(fileName, "PNG")) {
            QMessageBox::information(this, tr("Saved"), tr("QRコード画像を保存しました: %1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("QRコード画像の保存に失敗しました。"));
        }
    }
}
