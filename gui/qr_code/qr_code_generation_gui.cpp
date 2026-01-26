#include "qr_code_generation_gui.h"

#include "core/qr_tool/content_generator.h"
#include "core/qr_tool/qrcodegen.hpp"
#include "ui_qr_code_generation_gui.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QVariantMap>

using qrcodegen::QrCode;
using qrcodegen::QrSegment;

QRCodeGenerationGUI::QRCodeGenerationGUI(QWidget *parent)
    : GuiTool(parent), ui(new Ui::QRCodeGenerationGUI)
{
    ui->setupUi(this);

    parameterStack = ui->parameterStackedWidget;

    initializeCategories();
    setupParameterWidgets();

    connect(ui->categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &QRCodeGenerationGUI::onCategoryChanged);
    connect(ui->generateButton, &QPushButton::clicked, this,
            &QRCodeGenerationGUI::onGenerateClicked);
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
    parameterStack->addWidget(createTextWidget());     // Index 1
    parameterStack->addWidget(createUrlWidget());      // Index 2
    parameterStack->addWidget(createEmailWidget());    // Index 3
    parameterStack->addWidget(createPhoneWidget());    // Index 4
    parameterStack->addWidget(createSmsWidget());      // Index 5
    parameterStack->addWidget(createWifiWidget());     // Index 6
    parameterStack->addWidget(createContactWidget());  // Index 7
    parameterStack->addWidget(createCalendarWidget()); // Index 8
    parameterStack->addWidget(createGeoWidget());      // Index 9
}

QWidget *QRCodeGenerationGUI::createTextWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *textEdit = new QTextEdit();
    textEdit->setPlaceholderText(tr("Enter text..."));
    textEdit->setMinimumHeight(100);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addRow(tr("Text:"), textEdit);

    // エラーラベル作成
    auto *errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: red; font-size: 12px;");
    errorLabel->setVisible(false);
    errorLabels["text_content_error"] = errorLabel;
    layout->addRow("", errorLabel);

    parameterWidgets["text_content"] = textEdit;
    connect(textEdit, &QTextEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createUrlWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *urlEdit = new QLineEdit();
    urlEdit->setPlaceholderText(tr("Enter URL..."));
    urlEdit->setMinimumWidth(200);
    urlEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("URL:"), urlEdit);

    // エラーラベル作成
    auto *errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: red; font-size: 12px;");
    errorLabel->setVisible(false);
    errorLabels["url_content_error"] = errorLabel;
    layout->addRow("", errorLabel);

    parameterWidgets["url_content"] = urlEdit;
    connect(urlEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createEmailWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText(tr("Enter email address..."));
    emailEdit->setMinimumWidth(200);
    emailEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Email:"), emailEdit);

    // エラーラベル作成
    auto *errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: red; font-size: 12px;");
    errorLabel->setVisible(false);
    errorLabels["email_address_error"] = errorLabel;
    layout->addRow("", errorLabel);

    parameterWidgets["email_address"] = emailEdit;
    connect(emailEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createPhoneWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(tr("Enter phone number..."));
    phoneEdit->setMinimumWidth(200);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Phone:"), phoneEdit);

    // エラーラベル作成
    auto *errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: red; font-size: 12px;");
    errorLabel->setVisible(false);
    errorLabels["phone_number_error"] = errorLabel;
    layout->addRow("", errorLabel);

    parameterWidgets["phone_number"] = phoneEdit;
    connect(phoneEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createSmsWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(tr("Enter phone number..."));
    phoneEdit->setMinimumWidth(200);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Phone:"), phoneEdit);

    // 電話番号エラーラベル作成
    auto *phoneErrorLabel = new QLabel();
    phoneErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    phoneErrorLabel->setVisible(false);
    errorLabels["sms_phone_error"] = phoneErrorLabel;
    layout->addRow("", phoneErrorLabel);

    auto *messageEdit = new QTextEdit();
    messageEdit->setPlaceholderText(tr("Enter message..."));
    messageEdit->setMinimumHeight(80);
    messageEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addRow(tr("Message:"), messageEdit);

    // メッセージエラーラベル作成
    auto *messageErrorLabel = new QLabel();
    messageErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    messageErrorLabel->setVisible(false);
    errorLabels["sms_message_error"] = messageErrorLabel;
    layout->addRow("", messageErrorLabel);

    parameterWidgets["sms_phone"] = phoneEdit;
    parameterWidgets["sms_message"] = messageEdit;
    connect(phoneEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(messageEdit, &QTextEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createWifiWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *ssidEdit = new QLineEdit();
    ssidEdit->setPlaceholderText(tr("Enter SSID..."));
    ssidEdit->setMinimumWidth(200);
    ssidEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("SSID:"), ssidEdit);

    // SSIDエラーラベル作成
    auto *ssidErrorLabel = new QLabel();
    ssidErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    ssidErrorLabel->setVisible(false);
    errorLabels["wifi_ssid_error"] = ssidErrorLabel;
    layout->addRow("", ssidErrorLabel);

    auto *passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText(tr("Enter password..."));
    passwordEdit->setMinimumWidth(200);
    passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Password:"), passwordEdit);

    // パスワードエラーラベル作成
    auto *passwordErrorLabel = new QLabel();
    passwordErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    passwordErrorLabel->setVisible(false);
    errorLabels["wifi_password_error"] = passwordErrorLabel;
    layout->addRow("", passwordErrorLabel);

    auto *securityCombo = new QComboBox();
    securityCombo->addItem("WPA");
    securityCombo->addItem("WEP");
    securityCombo->addItem("nopass");
    layout->addRow(tr("Security:"), securityCombo);

    auto *hiddenCheck = new QCheckBox();
    layout->addRow(tr("Hidden Network"), hiddenCheck);

    parameterWidgets["wifi_ssid"] = ssidEdit;
    parameterWidgets["wifi_password"] = passwordEdit;
    parameterWidgets["wifi_security"] = securityCombo;
    parameterWidgets["wifi_hidden"] = hiddenCheck;
    connect(ssidEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(passwordEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(securityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createContactWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);

    auto *nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText(tr("Enter name..."));
    layout->addRow(tr("Name:"), nameEdit);

    // 名前エラーラベル作成
    auto *nameErrorLabel = new QLabel();
    nameErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    nameErrorLabel->setVisible(false);
    errorLabels["contact_name_error"] = nameErrorLabel;
    layout->addRow("", nameErrorLabel);

    auto *phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText(tr("Enter phone..."));
    phoneEdit->setMinimumWidth(200);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Phone:"), phoneEdit);

    // 電話エラーラベル作成
    auto *phoneErrorLabel = new QLabel();
    phoneErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    phoneErrorLabel->setVisible(false);
    errorLabels["contact_phone_error"] = phoneErrorLabel;
    layout->addRow("", phoneErrorLabel);

    auto *emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText(tr("Enter email..."));
    emailEdit->setMinimumWidth(200);
    emailEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Email:"), emailEdit);

    // メールエラーラベル作成
    auto *emailErrorLabel = new QLabel();
    emailErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    emailErrorLabel->setVisible(false);
    errorLabels["contact_email_error"] = emailErrorLabel;
    layout->addRow("", emailErrorLabel);

    parameterWidgets["contact_name"] = nameEdit;
    parameterWidgets["contact_phone"] = phoneEdit;
    parameterWidgets["contact_email"] = emailEdit;
    connect(nameEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(phoneEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(emailEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createCalendarWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *summaryEdit = new QLineEdit();
    summaryEdit->setPlaceholderText(tr("Enter event title..."));
    summaryEdit->setMinimumWidth(200);
    summaryEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Event:"), summaryEdit);

    // イベントタイトルエラーラベル作成
    auto *summaryErrorLabel = new QLabel();
    summaryErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    summaryErrorLabel->setVisible(false);
    errorLabels["cal_summary_error"] = summaryErrorLabel;
    layout->addRow("", summaryErrorLabel);

    auto *startEdit = new QDateTimeEdit();
    startEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    startEdit->setDateTime(QDateTime::currentDateTime());
    startEdit->setMinimumWidth(200);
    startEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    startEdit->setCalendarPopup(true);
    layout->addRow(tr("Start:"), startEdit);

    // 開始時刻エラーラベル作成
    auto *startErrorLabel = new QLabel();
    startErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    startErrorLabel->setVisible(false);
    errorLabels["cal_start_error"] = startErrorLabel;
    layout->addRow("", startErrorLabel);

    auto *endEdit = new QDateTimeEdit();
    endEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    endEdit->setDateTime(QDateTime::currentDateTime().addSecs(3600));
    endEdit->setMinimumWidth(200);
    endEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    endEdit->setCalendarPopup(true);
    layout->addRow(tr("End:"), endEdit);

    // 終了時刻エラーラベル作成
    auto *endErrorLabel = new QLabel();
    endErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    endErrorLabel->setVisible(false);
    errorLabels["cal_end_error"] = endErrorLabel;
    layout->addRow("", endErrorLabel);

    parameterWidgets["cal_summary"] = summaryEdit;
    parameterWidgets["cal_start"] = startEdit;
    parameterWidgets["cal_end"] = endEdit;
    connect(summaryEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(startEdit, &QDateTimeEdit::dateTimeChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(endEdit, &QDateTimeEdit::dateTimeChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createGeoWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *latEdit = new QLineEdit();
    latEdit->setPlaceholderText(tr("Enter latitude..."));
    latEdit->setMinimumWidth(200);
    latEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Latitude:"), latEdit);

    // 緯度エラーラベル作成
    auto *latErrorLabel = new QLabel();
    latErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    latErrorLabel->setVisible(false);
    errorLabels["geo_lat_error"] = latErrorLabel;
    layout->addRow("", latErrorLabel);

    auto *lngEdit = new QLineEdit();
    lngEdit->setPlaceholderText(tr("Enter longitude..."));
    lngEdit->setMinimumWidth(200);
    lngEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addRow(tr("Longitude:"), lngEdit);

    // 経度エラーラベル作成
    auto *lngErrorLabel = new QLabel();
    lngErrorLabel->setStyleSheet("color: red; font-size: 12px;");
    lngErrorLabel->setVisible(false);
    errorLabels["geo_lng_error"] = lngErrorLabel;
    layout->addRow("", lngErrorLabel);

    parameterWidgets["geo_lat"] = latEdit;
    parameterWidgets["geo_lng"] = lngEdit;
    connect(latEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(lngEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

void QRCodeGenerationGUI::onCategoryChanged(int index)
{
    // Map combo box index to QRCodeType and parameterStack index
    switch (index) {
    case 0: // Text
        currentType = Text;
        parameterStack->setCurrentIndex(1);
        break;
    case 1: // URL
        currentType = Url;
        parameterStack->setCurrentIndex(2);
        break;
    case 2: // Email
        currentType = Email;
        parameterStack->setCurrentIndex(3);
        break;
    case 3: // Phone Number
        currentType = Phone;
        parameterStack->setCurrentIndex(4);
        break;
    case 4: // SMS
        currentType = Sms;
        parameterStack->setCurrentIndex(5);
        break;
    case 5: // WiFi
        currentType = Wifi;
        parameterStack->setCurrentIndex(6);
        break;
    case 6: // Contact
        currentType = Contact;
        parameterStack->setCurrentIndex(7);
        break;
    case 7: // Calendar
        currentType = Calendar;
        parameterStack->setCurrentIndex(8);
        break;
    case 8: // Geo Location
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
    for (auto *widget : parameterWidgets.values()) {
        if (auto *lineEdit = qobject_cast<QLineEdit *>(widget)) {
            lineEdit->clear();
        } else if (auto *textEdit = qobject_cast<QTextEdit *>(widget)) {
            textEdit->clear();
        } else if (auto *combo = qobject_cast<QComboBox *>(widget)) {
            combo->setCurrentIndex(0);
        } else if (auto *checkBox = qobject_cast<QCheckBox *>(widget)) {
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
        auto const *edit = qobject_cast<QTextEdit *>(parameterWidgets["text_content"]);
        if ((edit == nullptr) || edit->toPlainText().trimmed().isEmpty()) {
            showValidationError("text_content_error", tr("Text cannot be empty"));
            isValid = false;
        }
        break;
    }

    case Url: {
        auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["url_content"]);
        if ((edit == nullptr) || edit->text().trimmed().isEmpty()) {
            showValidationError("url_content_error", tr("URL cannot be empty"));
            isValid = false;
        } else if (!edit->text().startsWith("http://") && !edit->text().startsWith("https://")) {
            showValidationError("url_content_error", tr("URL must start with http:// or https://"));
            isValid = false;
        }
        break;
    }

    case Email: {
        auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["email_address"]);
        if ((edit == nullptr) || edit->text().trimmed().isEmpty()) {
            showValidationError("email_address_error", tr("Email cannot be empty"));
            isValid = false;
        } else if (!edit->text().contains("@")) {
            showValidationError("email_address_error", tr("Invalid email format"));
            isValid = false;
        }
        break;
    }

    case Phone: {
        auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["phone_number"]);
        if ((edit == nullptr) || edit->text().trimmed().isEmpty()) {
            showValidationError("phone_number_error", tr("Phone number cannot be empty"));
            isValid = false;
        } else if (!isValidPhoneNumber(edit->text())) {
            showValidationError("phone_number_error", tr("Invalid phone number format"));
            isValid = false;
        }
        break;
    }

    case Sms: {
        auto const *phoneEdit = qobject_cast<QLineEdit *>(parameterWidgets["sms_phone"]);
        auto const *messageEdit = qobject_cast<QTextEdit *>(parameterWidgets["sms_message"]);

        if ((phoneEdit == nullptr) || phoneEdit->text().trimmed().isEmpty()) {
            showValidationError("sms_phone_error", tr("Phone number cannot be empty"));
            isValid = false;
        } else if (!isValidPhoneNumber(phoneEdit->text())) {
            showValidationError("sms_phone_error", tr("Invalid phone number format"));
            isValid = false;
        }
        if ((messageEdit == nullptr) || messageEdit->toPlainText().trimmed().isEmpty()) {
            showValidationError("sms_message_error", tr("Message cannot be empty"));
            isValid = false;
        }
        break;
    }

    case Wifi: {
        auto const *ssidEdit = qobject_cast<QLineEdit *>(parameterWidgets["wifi_ssid"]);
        auto const *passwordEdit = qobject_cast<QLineEdit *>(parameterWidgets["wifi_password"]);
        auto const *securityCombo = qobject_cast<QComboBox *>(parameterWidgets["wifi_security"]);

        if ((ssidEdit == nullptr) || ssidEdit->text().trimmed().isEmpty()) {
            showValidationError("wifi_ssid_error", tr("SSID cannot be empty"));
            isValid = false;
        }

        // WPAまたはWEPが選択されている場合、パスワードが必須
        if ((securityCombo != nullptr) &&
            (securityCombo->currentText() == "WPA" || securityCombo->currentText() == "WEP")) {
            if ((passwordEdit == nullptr) || passwordEdit->text().trimmed().isEmpty()) {
                showValidationError("wifi_password_error",
                                    tr("Password is required for WPA/WEP security"));
                isValid = false;
            }
        }
        break;
    }

    case Contact: {
        auto const *nameEdit = qobject_cast<QLineEdit *>(parameterWidgets["contact_name"]);
        auto const *phoneEdit = qobject_cast<QLineEdit *>(parameterWidgets["contact_phone"]);
        auto const *emailEdit = qobject_cast<QLineEdit *>(parameterWidgets["contact_email"]);

        if ((nameEdit == nullptr) || nameEdit->text().trimmed().isEmpty()) {
            showValidationError("contact_name_error", tr("Name cannot be empty"));
            isValid = false;
        }

        // 少なくとも電話番号かメールアドレスのどちらかは必須
        bool const hasPhone = (phoneEdit != nullptr) && !phoneEdit->text().trimmed().isEmpty();
        bool const hasEmail = (emailEdit != nullptr) && !emailEdit->text().trimmed().isEmpty();

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
        auto const *summaryEdit = qobject_cast<QLineEdit *>(parameterWidgets["cal_summary"]);
        auto const *startEdit = qobject_cast<QDateTimeEdit *>(parameterWidgets["cal_start"]);
        auto const *endEdit = qobject_cast<QDateTimeEdit *>(parameterWidgets["cal_end"]);

        if ((summaryEdit == nullptr) || summaryEdit->text().trimmed().isEmpty()) {
            showValidationError("cal_summary_error", tr("Event title cannot be empty"));
            isValid = false;
        }

        if (startEdit == nullptr) {
            showValidationError("cal_start_error", tr("Start date/time cannot be empty"));
            isValid = false;
        }

        if (endEdit == nullptr) {
            showValidationError("cal_end_error", tr("End date/time cannot be empty"));
            isValid = false;
        }

        // 開始時間が終了時間より後でないかチェック
        if ((startEdit != nullptr) && (endEdit != nullptr) &&
            startEdit->dateTime() >= endEdit->dateTime()) {
            showValidationError("cal_end_error", tr("End time must be after start time"));
            isValid = false;
        }
        break;
    }

    case Geo: {
        auto const *latEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lat"]);
        auto const *lngEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lng"]);

        if ((latEdit == nullptr) || latEdit->text().trimmed().isEmpty()) {
            showValidationError("geo_lat_error", tr("Latitude cannot be empty"));
            isValid = false;
        } else {
            bool ok;
            double const lat = latEdit->text().toDouble(&ok);
            if (!ok || lat < -90.0 || lat > 90.0) {
                showValidationError("geo_lat_error", tr("Latitude must be between -90 and 90"));
                isValid = false;
            }
        }

        if ((lngEdit == nullptr) || lngEdit->text().trimmed().isEmpty()) {
            showValidationError("geo_lng_error", tr("Longitude cannot be empty"));
            isValid = false;
        } else {
            bool ok;
            double const lng = lngEdit->text().toDouble(&ok);
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

void QRCodeGenerationGUI::showValidationError(const QString &fieldKey, const QString &message)
{
    if (QLabel *errorLabel = errorLabels.value(fieldKey)) {
        errorLabel->setText(message);
        errorLabel->setVisible(true);
    }
}

void QRCodeGenerationGUI::clearValidationErrors()
{
    for (auto *label : errorLabels.values()) {
        label->setVisible(false);
        label->clear();
    }
}

bool QRCodeGenerationGUI::isValidPhoneNumber(const QString &phone)
{
    if (phone.trimmed().isEmpty()) {
        return false;
    }

    QString const cleanPhone = phone.trimmed();

    // 国際番号形式（+81-xxx-xxxx-xxxx, +81xxxxxxxxxx）
    QRegularExpression const internationalPattern(
        R"(^\+\d{1,3}[-\s]?\d{1,4}[-\s]?\d{4}[-\s]?\d{4}$)");
    if (internationalPattern.match(cleanPhone).hasMatch()) {
        return true;
    }

    // 日本の国内番号形式（0xx-xxxx-xxxx, 0xxxxxxxxxx）
    QRegularExpression const domesticPattern(R"(^0\d{1,4}[-\s]?\d{2,4}[-\s]?\d{4}$)");
    if (domesticPattern.match(cleanPhone).hasMatch()) {
        return true;
    }

    // 数字のみ（10-11桁）
    QRegularExpression const digitOnlyPattern(R"(^\d{10,11}$)");
    return digitOnlyPattern.match(cleanPhone).hasMatch();
}

QString QRCodeGenerationGUI::generateQRCodeContent()
{
    QVariantMap params;
    QString type;

    switch (currentType) {
    case Text: {
        if (auto const *edit = qobject_cast<QTextEdit *>(parameterWidgets["text_content"])) {
            params["text"] = edit->toPlainText();
        }
        type = "text";
        break;
    }

    case Url: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["url_content"])) {
            params["url"] = edit->text();
        }
        type = "url";
        break;
    }

    case Email: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["email_address"])) {
            params["email"] = edit->text();
        }
        type = "email";
        break;
    }

    case Phone: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["phone_number"])) {
            params["number"] = edit->text();
        }
        type = "phone";
        break;
    }

    case Sms: {
        if (auto const *phoneEdit = qobject_cast<QLineEdit *>(parameterWidgets["sms_phone"])) {
            params["number"] = phoneEdit->text();
        }
        if (auto const *messageEdit = qobject_cast<QTextEdit *>(parameterWidgets["sms_message"])) {
            params["text"] = messageEdit->toPlainText();
        }
        type = "sms";
        break;
    }

    case Contact: {
        if (auto const *nameEdit = qobject_cast<QLineEdit *>(parameterWidgets["contact_name"])) {
            params["name"] = nameEdit->text();
        }
        if (auto const *phoneEdit = qobject_cast<QLineEdit *>(parameterWidgets["contact_phone"])) {
            params["phone"] = phoneEdit->text();
        }
        if (auto const *emailEdit = qobject_cast<QLineEdit *>(parameterWidgets["contact_email"])) {
            params["email"] = emailEdit->text();
        }
        type = "contact";
        break;
    }

    case Calendar: {
        if (auto const *summaryEdit = qobject_cast<QLineEdit *>(parameterWidgets["cal_summary"])) {
            params["summary"] = summaryEdit->text();
        }
        if (auto const *startEdit = qobject_cast<QDateTimeEdit *>(parameterWidgets["cal_start"])) {
            params["start"] = startEdit->dateTime().toString("yyyyMMddThhmmss");
        }
        if (auto const *endEdit = qobject_cast<QDateTimeEdit *>(parameterWidgets["cal_end"])) {
            params["end"] = endEdit->dateTime().toString("yyyyMMddThhmmss");
        }
        type = "calendar";
        break;
    }

    case Geo: {
        if (auto const *latEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lat"])) {
            params["lat"] = latEdit->text();
        }
        if (auto const *lngEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lng"])) {
            params["lng"] = lngEdit->text();
        }
        type = "geo";
        break;
    }

    case Wifi: {
        if (auto const *ssidEdit = qobject_cast<QLineEdit *>(parameterWidgets["wifi_ssid"])) {
            params["ssid"] = ssidEdit->text();
        }
        if (auto const *passEdit = qobject_cast<QLineEdit *>(parameterWidgets["wifi_password"])) {
            params["password"] = passEdit->text();
        }
        if (auto const *securityCombo =
                qobject_cast<QComboBox *>(parameterWidgets["wifi_security"])) {
            params["type"] = securityCombo->currentText();
        }
        if (auto const *hiddenCheck = qobject_cast<QCheckBox *>(parameterWidgets["wifi_hidden"])) {
            params["hidden"] = hiddenCheck->isChecked() ? "true" : "false";
        }
        type = "wifi";
        break;
    }
    }

    // バリデーションチェック（後で実装される部分なのでシンプルに）
    if (params.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please fill in required fields"));
        return {};
    }

    ContentGenerator const generator;
    return ContentGenerator::generate(type, params);
}

void QRCodeGenerationGUI::updateGenerateButtonState()
{
    bool enabled = false;

    switch (currentType) {
    case Text: {
        if (auto const *edit = qobject_cast<QTextEdit *>(parameterWidgets["text_content"])) {
            enabled = !edit->toPlainText().trimmed().isEmpty();
        }
        break;
    }

    case Url: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["url_content"])) {
            enabled = !edit->text().trimmed().isEmpty();
        }
        break;
    }

    case Email: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["email_address"])) {
            enabled = !edit->text().trimmed().isEmpty();
        }
        break;
    }

    case Phone: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["phone_number"])) {
            enabled = !edit->text().trimmed().isEmpty();
        }
        break;
    }

    case Sms: {
        auto const *phoneEdit = qobject_cast<QLineEdit *>(parameterWidgets["sms_phone"]);
        auto const *messageEdit = qobject_cast<QTextEdit *>(parameterWidgets["sms_message"]);
        enabled = (phoneEdit != nullptr) && (messageEdit != nullptr) &&
                  !phoneEdit->text().trimmed().isEmpty() &&
                  !messageEdit->toPlainText().trimmed().isEmpty();
        break;
    }

    case Wifi: {
        auto const *ssidEdit = qobject_cast<QLineEdit *>(parameterWidgets["wifi_ssid"]);
        auto const *securityCombo = qobject_cast<QComboBox *>(parameterWidgets["wifi_security"]);
        auto const *passwordEdit = qobject_cast<QLineEdit *>(parameterWidgets["wifi_password"]);

        if ((ssidEdit != nullptr) && (securityCombo != nullptr)) {
            enabled = !ssidEdit->text().trimmed().isEmpty();

            // WPA or WEP選択時はパスワード必須
            QString const security = securityCombo->currentText();
            if ((security == "WPA" || security == "WEP") && (passwordEdit != nullptr)) {
                enabled = enabled && !passwordEdit->text().trimmed().isEmpty();
            }
        }
        break;
    }

    case Contact: {
        auto const *nameEdit = qobject_cast<QLineEdit *>(parameterWidgets["contact_name"]);
        auto const *phoneEdit = qobject_cast<QLineEdit *>(parameterWidgets["contact_phone"]);
        auto const *emailEdit = qobject_cast<QLineEdit *>(parameterWidgets["contact_email"]);

        if (nameEdit != nullptr) {
            bool const hasName = !nameEdit->text().trimmed().isEmpty();
            bool hasPhoneOrEmail = false;

            if ((phoneEdit != nullptr) && !phoneEdit->text().trimmed().isEmpty()) {
                hasPhoneOrEmail = true;
            }
            if ((emailEdit != nullptr) && !emailEdit->text().trimmed().isEmpty()) {
                hasPhoneOrEmail = true;
            }

            enabled = hasName && hasPhoneOrEmail;
        }
        break;
    }

    case Calendar: {
        auto const *summaryEdit = qobject_cast<QLineEdit *>(parameterWidgets["cal_summary"]);
        auto const *startEdit = qobject_cast<QDateTimeEdit *>(parameterWidgets["cal_start"]);
        auto const *endEdit = qobject_cast<QDateTimeEdit *>(parameterWidgets["cal_end"]);

        enabled = (summaryEdit != nullptr) && (startEdit != nullptr) && (endEdit != nullptr) &&
                  !summaryEdit->text().trimmed().isEmpty();
        break;
    }

    case Geo: {
        auto const *latEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lat"]);
        auto const *lngEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lng"]);

        enabled = (latEdit != nullptr) && (lngEdit != nullptr) &&
                  !latEdit->text().trimmed().isEmpty() && !lngEdit->text().trimmed().isEmpty();
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

    QString const content = generateQRCodeContent();

    if (content.isEmpty()) {
        return;
    }

    // Display content preview
    ui->contentPreviewEdit->setPlainText(content);

    try {
        // QRコード生成
        const QrCode qr = QrCode::encodeText(content.toUtf8().constData(), QrCode::Ecc::LOW);

        int const scale = 8; // 自由に設定させるかは要検討

        int const size = qr.getSize();
        QImage image(size * scale, size * scale, QImage::Format_RGB32);
        image.fill(Qt::white);

        // QRコード描画
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (qr.getModule(x, y)) {
                    for (int dy = 0; dy < scale; dy++) {
                        for (int dx = 0; dx < scale; dx++) {
                            image.setPixel((x * scale) + dx, (y * scale) + dy, qRgb(0, 0, 0));
                        }
                    }
                }
            }
        }

        // 生成されたQRコード画像をメンバ変数に保存
        currentQRImage = image;

        // QLabelのサイズに合わせて画像をリサイズ（アスペクト比を保持）
        QSize labelSize = ui->qrCodeLabel->size();
        // マージンを考慮してサイズを少し小さくする
        labelSize -= QSize(10, 10);

        QPixmap const pixmap = QPixmap::fromImage(image);
        // アスペクト比を保持しながら、ラベルサイズに収まるようにスケーリング
        QPixmap const scaledPixmap =
            pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        ui->qrCodeLabel->setPixmap(scaledPixmap);
        ui->qrCodeLabel->show();

        // Enable output buttons
        ui->copyButton->setEnabled(true);
        ui->saveButton->setEnabled(true);

    } catch (const std::exception &e) {
        // QRコード生成時のエラーをキャッチ
        QMessageBox::critical(
            this, tr("Error"),
            tr("Failed to generate QR code:\n%1\n\n"
               "Please try reducing the amount of data or simplifying the content.")
                .arg(e.what()));

        // エラー時はボタンを無効化
        ui->copyButton->setEnabled(false);
        ui->saveButton->setEnabled(false);
    }
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
    QMessageBox::information(this, tr("Copied"),
                             tr("QRコード画像をクリップボードにコピーしました。"));
}

void QRCodeGenerationGUI::onSaveClicked()
{
    // 画像が生成されているかチェック
    if (currentQRImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("QRコードが生成されていません。"));
        return;
    }

    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save QR Code"), "", tr("PNG Files (*.png)"));
    if (!fileName.isEmpty()) {
        // ファイル拡張子がない場合は".png"を自動追加
        if (!fileName.toLower().endsWith(".png")) {
            fileName += ".png";
        }

        // 実際にファイル保存
        if (currentQRImage.save(fileName, "PNG")) {
            QMessageBox::information(this, tr("Saved"),
                                     tr("QRコード画像を保存しました: %1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("QRコード画像の保存に失敗しました。"));
        }
    }
}

void QRCodeGenerationGUI::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // QRコードが既に生成されている場合のみ再描画
    if (!currentQRImage.isNull()) {
        refreshQRCodeDisplay();
    }
}

void QRCodeGenerationGUI::refreshQRCodeDisplay()
{
    if (currentQRImage.isNull()) {
        return;
    }

    // QLabelのサイズに合わせて画像をリサイズ（アスペクト比を保持）
    QSize labelSize = ui->qrCodeLabel->size();
    // マージンを考慮してサイズを少し小さくする
    labelSize -= QSize(10, 10);

    QPixmap const pixmap = QPixmap::fromImage(currentQRImage);
    // アスペクト比を保持しながら、ラベルサイズに収まるようにスケーリング
    QPixmap const scaledPixmap =
        pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->qrCodeLabel->setPixmap(scaledPixmap);
}
