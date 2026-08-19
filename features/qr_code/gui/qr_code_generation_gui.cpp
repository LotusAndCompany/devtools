#include "qr_code_generation_gui.h"

#include "features/framework/gui/design_system.h"
#include "features/qr_code/core/content_generator.h"
#include "features/qr_code/core/qrcodegen.hpp"

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
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QVariantMap>

using qrcodegen::QrCode;

namespace {
constexpr int CATEGORY_ITEM_COUNT = 9;
constexpr int QR_SCALE = 8;
} // namespace

QRCodeGenerationGUI::QRCodeGenerationGUI(QWidget *parent) : GuiTool(parent)
{
    buildUi();

    setupParameterWidgets();

    connect(categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &QRCodeGenerationGUI::onCategoryChanged);
    connect(generateButton, &QPushButton::clicked, this, &QRCodeGenerationGUI::onGenerateClicked);
    connect(clearButton, &QPushButton::clicked, this, &QRCodeGenerationGUI::onClearClicked);
    connect(copyButton, &QPushButton::clicked, this, &QRCodeGenerationGUI::onCopyClicked);
    connect(saveButton, &QPushButton::clicked, this, &QRCodeGenerationGUI::onSaveClicked);

    // Set initial category
    onCategoryChanged(0);
}

// NOLINTNEXTLINE(readability-function-size)
void QRCodeGenerationGUI::buildUi()
{
    auto *horizontalLayoutMain = new QHBoxLayout(this);
    auto *verticalLayoutLeft = new QVBoxLayout();
    DevTools::Ui::applyPageLayout(horizontalLayoutMain);
    DevTools::Ui::applyPanelLayout(verticalLayoutLeft);

    // Category group box
    categoryGroupBox = new QGroupBox(this);
    auto *categoryLayout = new QVBoxLayout(categoryGroupBox);
    DevTools::Ui::applyPanelLayout(categoryLayout);
    categoryComboBox = new QComboBox(categoryGroupBox);
    for (int i = 0; i < CATEGORY_ITEM_COUNT; ++i) {
        categoryComboBox->addItem(QString());
    }
    categoryLayout->addWidget(categoryComboBox);
    verticalLayoutLeft->addWidget(categoryGroupBox);

    // Parameters group box
    parametersGroupBox = new QGroupBox(this);
    auto *paramsLayout = new QVBoxLayout(parametersGroupBox);
    DevTools::Ui::applyPanelLayout(paramsLayout);
    parameterStackedWidget = new QStackedWidget(parametersGroupBox);
    auto *emptyPage = new QWidget();
    parameterStackedWidget->addWidget(emptyPage); // index 0
    parameterStackedWidget->setCurrentIndex(0);
    paramsLayout->addWidget(parameterStackedWidget);
    verticalLayoutLeft->addWidget(parametersGroupBox);

    // Button row
    auto *buttonLayout = new QHBoxLayout();
    generateButton = new QPushButton(this);
    DevTools::Ui::configureCompactButton(generateButton);
    clearButton = new QPushButton(this);
    DevTools::Ui::configureCompactButton(clearButton);
    buttonLayout->addWidget(generateButton);
    buttonLayout->addWidget(clearButton);
    DevTools::Ui::configureActionBar(buttonLayout, DevTools::Ui::ActionBarAlignment::Trailing);
    verticalLayoutLeft->addLayout(buttonLayout);

    horizontalLayoutMain->addLayout(verticalLayoutLeft);

    // Right side: output
    auto *verticalLayoutRight = new QVBoxLayout();
    DevTools::Ui::applyPanelLayout(verticalLayoutRight);
    outputGroupBox = new QGroupBox(this);
    auto *outputLayout = new QVBoxLayout(outputGroupBox);
    DevTools::Ui::applyPanelLayout(outputLayout);

    qrCodeLabel = new QLabel(outputGroupBox);
    DevTools::Ui::configurePreviewSurface(qrCodeLabel);
    outputLayout->addWidget(qrCodeLabel);

    auto *outputButtonLayout = new QHBoxLayout();
    copyButton = new QPushButton(outputGroupBox);
    DevTools::Ui::configureCompactButton(copyButton);
    copyButton->setEnabled(false);
    saveButton = new QPushButton(outputGroupBox);
    DevTools::Ui::configureCompactButton(saveButton);
    saveButton->setEnabled(false);
    outputButtonLayout->addWidget(copyButton);
    outputButtonLayout->addWidget(saveButton);
    DevTools::Ui::configureActionBar(outputButtonLayout,
                                     DevTools::Ui::ActionBarAlignment::Trailing);
    outputLayout->addLayout(outputButtonLayout);

    contentPreviewEdit = DevTools::Ui::createPlainTextEdit(outputGroupBox);
    DevTools::Ui::configurePreviewEditor(contentPreviewEdit);
    contentPreviewEdit->setReadOnly(true);
    outputLayout->addWidget(contentPreviewEdit);

    verticalLayoutRight->addWidget(outputGroupBox);
    horizontalLayoutMain->addLayout(verticalLayoutRight);
    DevTools::Ui::configureEqualLayout(horizontalLayoutMain);

    retranslateUi();
}

void QRCodeGenerationGUI::retranslateUi()
{
    setWindowTitle(tr("QR Code Generation"));
    categoryGroupBox->setTitle(tr("QR Code Type"));
    parametersGroupBox->setTitle(tr("Parameters"));
    outputGroupBox->setTitle(tr("QR Code Output"));

    const QStringList categoryItems = {
        tr("Text"), tr("URL"),     tr("Email"),    tr("Phone Number"), tr("SMS"),
        tr("WiFi"), tr("Contact"), tr("Calendar"), tr("Geo Location"),
    };
    for (int i = 0; i < categoryItems.size() && i < categoryComboBox->count(); ++i) {
        categoryComboBox->setItemText(i, categoryItems[i]);
    }

    generateButton->setText(tr("Generate"));
    clearButton->setText(tr("Clear"));
    copyButton->setText(tr("Copy Image"));
    saveButton->setText(tr("Save Image"));
    qrCodeLabel->setText(tr("QR Code will appear here"));
    contentPreviewEdit->setPlaceholderText(tr("QR code content preview"));
}

void QRCodeGenerationGUI::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void QRCodeGenerationGUI::setupParameterWidgets()
{
    // Create and add parameter widgets for all types
    parameterStackedWidget->addWidget(createTextWidget());     // Index 1
    parameterStackedWidget->addWidget(createUrlWidget());      // Index 2
    parameterStackedWidget->addWidget(createEmailWidget());    // Index 3
    parameterStackedWidget->addWidget(createPhoneWidget());    // Index 4
    parameterStackedWidget->addWidget(createSmsWidget());      // Index 5
    parameterStackedWidget->addWidget(createWifiWidget());     // Index 6
    parameterStackedWidget->addWidget(createContactWidget());  // Index 7
    parameterStackedWidget->addWidget(createCalendarWidget()); // Index 8
    parameterStackedWidget->addWidget(createGeoWidget());      // Index 9
}

QWidget *QRCodeGenerationGUI::createTextWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    DevTools::Ui::configureFormLayout(layout);

    auto *textEdit = DevTools::Ui::createPlainTextEdit();
    textEdit->setPlaceholderText(tr("Enter text..."));
    DevTools::Ui::configureMultilineField(textEdit);
    layout->addRow(tr("Text:"), textEdit);

    // エラーラベル作成
    auto *errorLabel = DevTools::Ui::createErrorLabel(widget);
    errorLabels["text_content_error"] = errorLabel;
    layout->addRow("", errorLabel);

    parameterWidgets["text_content"] = textEdit;
    connect(textEdit, &QPlainTextEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createUrlWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    DevTools::Ui::configureFormLayout(layout);

    auto *urlEdit = DevTools::Ui::createLineEdit();
    urlEdit->setPlaceholderText(tr("Enter URL..."));
    DevTools::Ui::configureFormField(urlEdit);
    layout->addRow(tr("URL:"), urlEdit);

    // エラーラベル作成
    auto *errorLabel = DevTools::Ui::createErrorLabel(widget);
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
    DevTools::Ui::configureFormLayout(layout);

    auto *emailEdit = DevTools::Ui::createLineEdit();
    emailEdit->setPlaceholderText(tr("Enter email address..."));
    DevTools::Ui::configureFormField(emailEdit);
    layout->addRow(tr("Email:"), emailEdit);

    // エラーラベル作成
    auto *errorLabel = DevTools::Ui::createErrorLabel(widget);
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
    DevTools::Ui::configureFormLayout(layout);

    auto *phoneEdit = DevTools::Ui::createLineEdit();
    phoneEdit->setPlaceholderText(tr("Enter phone number..."));
    DevTools::Ui::configureFormField(phoneEdit);
    layout->addRow(tr("Phone:"), phoneEdit);

    // エラーラベル作成
    auto *errorLabel = DevTools::Ui::createErrorLabel(widget);
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
    DevTools::Ui::configureFormLayout(layout);

    auto *phoneEdit = DevTools::Ui::createLineEdit();
    phoneEdit->setPlaceholderText(tr("Enter phone number..."));
    DevTools::Ui::configureFormField(phoneEdit);
    layout->addRow(tr("Phone:"), phoneEdit);

    // 電話番号エラーラベル作成
    auto *phoneErrorLabel = DevTools::Ui::createErrorLabel(widget);
    errorLabels["sms_phone_error"] = phoneErrorLabel;
    layout->addRow("", phoneErrorLabel);

    auto *messageEdit = DevTools::Ui::createPlainTextEdit();
    messageEdit->setPlaceholderText(tr("Enter message..."));
    DevTools::Ui::configureMultilineField(messageEdit);
    layout->addRow(tr("Message:"), messageEdit);

    // メッセージエラーラベル作成
    auto *messageErrorLabel = DevTools::Ui::createErrorLabel(widget);
    errorLabels["sms_message_error"] = messageErrorLabel;
    layout->addRow("", messageErrorLabel);

    parameterWidgets["sms_phone"] = phoneEdit;
    parameterWidgets["sms_message"] = messageEdit;
    connect(phoneEdit, &QLineEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);
    connect(messageEdit, &QPlainTextEdit::textChanged, this,
            &QRCodeGenerationGUI::updateGenerateButtonState);

    return widget;
}

QWidget *QRCodeGenerationGUI::createWifiWidget()
{
    auto *widget = new QWidget();
    auto *layout = new QFormLayout(widget);
    DevTools::Ui::configureFormLayout(layout);

    auto *ssidEdit = DevTools::Ui::createLineEdit();
    ssidEdit->setPlaceholderText(tr("Enter SSID..."));
    DevTools::Ui::configureFormField(ssidEdit);
    layout->addRow(tr("SSID:"), ssidEdit);

    // SSIDエラーラベル作成
    auto *ssidErrorLabel = DevTools::Ui::createErrorLabel(widget);
    errorLabels["wifi_ssid_error"] = ssidErrorLabel;
    layout->addRow("", ssidErrorLabel);

    auto *passwordEdit = DevTools::Ui::createLineEdit();
    passwordEdit->setPlaceholderText(tr("Enter password..."));
    DevTools::Ui::configureFormField(passwordEdit);
    layout->addRow(tr("Password:"), passwordEdit);

    // パスワードエラーラベル作成
    auto *passwordErrorLabel = DevTools::Ui::createErrorLabel(widget);
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
    DevTools::Ui::configureFormLayout(layout);

    auto *nameEdit = DevTools::Ui::createLineEdit();
    nameEdit->setPlaceholderText(tr("Enter name..."));
    DevTools::Ui::configureFormField(nameEdit);
    layout->addRow(tr("Name:"), nameEdit);

    // 名前エラーラベル作成
    auto *nameErrorLabel = DevTools::Ui::createErrorLabel(widget);
    errorLabels["contact_name_error"] = nameErrorLabel;
    layout->addRow("", nameErrorLabel);

    auto *phoneEdit = DevTools::Ui::createLineEdit();
    phoneEdit->setPlaceholderText(tr("Enter phone..."));
    DevTools::Ui::configureFormField(phoneEdit);
    layout->addRow(tr("Phone:"), phoneEdit);

    // 電話エラーラベル作成
    auto *phoneErrorLabel = DevTools::Ui::createErrorLabel(widget);
    errorLabels["contact_phone_error"] = phoneErrorLabel;
    layout->addRow("", phoneErrorLabel);

    auto *emailEdit = DevTools::Ui::createLineEdit();
    emailEdit->setPlaceholderText(tr("Enter email..."));
    DevTools::Ui::configureFormField(emailEdit);
    layout->addRow(tr("Email:"), emailEdit);

    // メールエラーラベル作成
    auto *emailErrorLabel = DevTools::Ui::createErrorLabel(widget);
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
    DevTools::Ui::configureFormLayout(layout);

    auto *summaryEdit = DevTools::Ui::createLineEdit();
    summaryEdit->setPlaceholderText(tr("Enter event title..."));
    DevTools::Ui::configureFormField(summaryEdit);
    layout->addRow(tr("Event:"), summaryEdit);

    // イベントタイトルエラーラベル作成
    auto *summaryErrorLabel = DevTools::Ui::createErrorLabel(widget);
    errorLabels["cal_summary_error"] = summaryErrorLabel;
    layout->addRow("", summaryErrorLabel);

    auto *startEdit = new QDateTimeEdit();
    startEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    startEdit->setDateTime(QDateTime::currentDateTime());
    DevTools::Ui::configureFormField(startEdit);
    startEdit->setCalendarPopup(true);
    layout->addRow(tr("Start:"), startEdit);

    // 開始時刻エラーラベル作成
    auto *startErrorLabel = DevTools::Ui::createErrorLabel(widget);
    errorLabels["cal_start_error"] = startErrorLabel;
    layout->addRow("", startErrorLabel);

    auto *endEdit = new QDateTimeEdit();
    endEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    endEdit->setDateTime(QDateTime::currentDateTime().addSecs(3600));
    DevTools::Ui::configureFormField(endEdit);
    endEdit->setCalendarPopup(true);
    layout->addRow(tr("End:"), endEdit);

    // 終了時刻エラーラベル作成
    auto *endErrorLabel = DevTools::Ui::createErrorLabel(widget);
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
    DevTools::Ui::configureFormLayout(layout);

    auto *latEdit = DevTools::Ui::createLineEdit();
    latEdit->setPlaceholderText(tr("Enter latitude..."));
    DevTools::Ui::configureFormField(latEdit);
    layout->addRow(tr("Latitude:"), latEdit);

    // 緯度エラーラベル作成
    auto *latErrorLabel = DevTools::Ui::createErrorLabel(widget);
    errorLabels["geo_lat_error"] = latErrorLabel;
    layout->addRow("", latErrorLabel);

    auto *lngEdit = DevTools::Ui::createLineEdit();
    lngEdit->setPlaceholderText(tr("Enter longitude..."));
    DevTools::Ui::configureFormField(lngEdit);
    layout->addRow(tr("Longitude:"), lngEdit);

    // 経度エラーラベル作成
    auto *lngErrorLabel = DevTools::Ui::createErrorLabel(widget);
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
        currentType = QRCodeType::Text;
        parameterStackedWidget->setCurrentIndex(1);
        break;
    case 1: // URL
        currentType = QRCodeType::Url;
        parameterStackedWidget->setCurrentIndex(2);
        break;
    case 2: // Email
        currentType = QRCodeType::Email;
        parameterStackedWidget->setCurrentIndex(3);
        break;
    case 3: // Phone Number
        currentType = QRCodeType::Phone;
        parameterStackedWidget->setCurrentIndex(4);
        break;
    case 4: // SMS
        currentType = QRCodeType::Sms;
        parameterStackedWidget->setCurrentIndex(5);
        break;
    case 5: // WiFi
        currentType = QRCodeType::Wifi;
        parameterStackedWidget->setCurrentIndex(6);
        break;
    case 6: // Contact
        currentType = QRCodeType::Contact;
        parameterStackedWidget->setCurrentIndex(7);
        break;
    case 7: // Calendar
        currentType = QRCodeType::Calendar;
        parameterStackedWidget->setCurrentIndex(8);
        break;
    case 8: // Geo Location
        currentType = QRCodeType::Geo;
        parameterStackedWidget->setCurrentIndex(9);
        break;
    default:
        // Default to Text
        currentType = QRCodeType::Text;
        parameterStackedWidget->setCurrentIndex(1);
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
        } else if (auto *textEdit = qobject_cast<QPlainTextEdit *>(widget)) {
            textEdit->clear();
        } else if (auto *combo = qobject_cast<QComboBox *>(widget)) {
            combo->setCurrentIndex(0);
        } else if (auto *checkBox = qobject_cast<QCheckBox *>(widget)) {
            checkBox->setChecked(false);
        }
    }

    contentPreviewEdit->clear();

    // エラーラベルもクリア
    clearValidationErrors();
}

// NOLINTNEXTLINE(readability-function-size)
bool QRCodeGenerationGUI::validateCurrentType()
{
    clearValidationErrors();
    bool isValid = true;

    switch (currentType) {
    case QRCodeType::Text: {
        auto const *edit = qobject_cast<QPlainTextEdit *>(parameterWidgets["text_content"]);
        if ((edit == nullptr) || edit->toPlainText().trimmed().isEmpty()) {
            showValidationError("text_content_error", tr("Text cannot be empty"));
            isValid = false;
        }
        break;
    }

    case QRCodeType::Url: {
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

    case QRCodeType::Email: {
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

    case QRCodeType::Phone: {
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

    case QRCodeType::Sms: {
        auto const *phoneEdit = qobject_cast<QLineEdit *>(parameterWidgets["sms_phone"]);
        auto const *messageEdit = qobject_cast<QPlainTextEdit *>(parameterWidgets["sms_message"]);

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

    case QRCodeType::Wifi: {
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

    case QRCodeType::Contact: {
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

    case QRCodeType::Calendar: {
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

    case QRCodeType::Geo: {
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

// NOLINTNEXTLINE(readability-function-size)
QString QRCodeGenerationGUI::generateQRCodeContent()
{
    QVariantMap params;
    QString type;

    switch (currentType) {
    case QRCodeType::Text: {
        if (auto const *edit = qobject_cast<QPlainTextEdit *>(parameterWidgets["text_content"])) {
            params["text"] = edit->toPlainText();
        }
        type = "text";
        break;
    }

    case QRCodeType::Url: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["url_content"])) {
            params["url"] = edit->text();
        }
        type = "url";
        break;
    }

    case QRCodeType::Email: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["email_address"])) {
            params["email"] = edit->text();
        }
        type = "email";
        break;
    }

    case QRCodeType::Phone: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["phone_number"])) {
            params["number"] = edit->text();
        }
        type = "phone";
        break;
    }

    case QRCodeType::Sms: {
        if (auto const *phoneEdit = qobject_cast<QLineEdit *>(parameterWidgets["sms_phone"])) {
            params["number"] = phoneEdit->text();
        }
        if (auto const *messageEdit =
                qobject_cast<QPlainTextEdit *>(parameterWidgets["sms_message"])) {
            params["text"] = messageEdit->toPlainText();
        }
        type = "sms";
        break;
    }

    case QRCodeType::Contact: {
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

    case QRCodeType::Calendar: {
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

    case QRCodeType::Geo: {
        if (auto const *latEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lat"])) {
            params["lat"] = latEdit->text();
        }
        if (auto const *lngEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lng"])) {
            params["lng"] = lngEdit->text();
        }
        type = "geo";
        break;
    }

    case QRCodeType::Wifi: {
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

// NOLINTNEXTLINE(readability-function-size)
void QRCodeGenerationGUI::updateGenerateButtonState()
{
    bool enabled = false;

    switch (currentType) {
    case QRCodeType::Text: {
        if (auto const *edit = qobject_cast<QPlainTextEdit *>(parameterWidgets["text_content"])) {
            enabled = !edit->toPlainText().trimmed().isEmpty();
        }
        break;
    }

    case QRCodeType::Url: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["url_content"])) {
            enabled = !edit->text().trimmed().isEmpty();
        }
        break;
    }

    case QRCodeType::Email: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["email_address"])) {
            enabled = !edit->text().trimmed().isEmpty();
        }
        break;
    }

    case QRCodeType::Phone: {
        if (auto const *edit = qobject_cast<QLineEdit *>(parameterWidgets["phone_number"])) {
            enabled = !edit->text().trimmed().isEmpty();
        }
        break;
    }

    case QRCodeType::Sms: {
        auto const *phoneEdit = qobject_cast<QLineEdit *>(parameterWidgets["sms_phone"]);
        auto const *messageEdit = qobject_cast<QPlainTextEdit *>(parameterWidgets["sms_message"]);
        enabled = (phoneEdit != nullptr) && (messageEdit != nullptr) &&
                  !phoneEdit->text().trimmed().isEmpty() &&
                  !messageEdit->toPlainText().trimmed().isEmpty();
        break;
    }

    case QRCodeType::Wifi: {
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

    case QRCodeType::Contact: {
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

    case QRCodeType::Calendar: {
        auto const *summaryEdit = qobject_cast<QLineEdit *>(parameterWidgets["cal_summary"]);
        auto const *startEdit = qobject_cast<QDateTimeEdit *>(parameterWidgets["cal_start"]);
        auto const *endEdit = qobject_cast<QDateTimeEdit *>(parameterWidgets["cal_end"]);

        enabled = (summaryEdit != nullptr) && (startEdit != nullptr) && (endEdit != nullptr) &&
                  !summaryEdit->text().trimmed().isEmpty();
        break;
    }

    case QRCodeType::Geo: {
        auto const *latEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lat"]);
        auto const *lngEdit = qobject_cast<QLineEdit *>(parameterWidgets["geo_lng"]);

        enabled = (latEdit != nullptr) && (lngEdit != nullptr) &&
                  !latEdit->text().trimmed().isEmpty() && !lngEdit->text().trimmed().isEmpty();
        break;
    }
    }

    generateButton->setEnabled(enabled);
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
    contentPreviewEdit->setPlainText(content);

    try {
        // QRコード生成
        const QrCode qr = QrCode::encodeText(content.toUtf8().constData(), QrCode::Ecc::LOW);

        int const size = qr.getSize();
        QImage image(size * QR_SCALE, size * QR_SCALE, QImage::Format_RGB32);
        image.fill(Qt::white);

        // QRコード描画
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (qr.getModule(x, y)) {
                    for (int dy = 0; dy < QR_SCALE; dy++) {
                        for (int dx = 0; dx < QR_SCALE; dx++) {
                            image.setPixel((x * QR_SCALE) + dx, (y * QR_SCALE) + dy, qRgb(0, 0, 0));
                        }
                    }
                }
            }
        }

        // 生成されたQRコード画像をメンバ変数に保存
        currentQRImage = image;

        // QLabelのサイズに合わせて画像をリサイズ（アスペクト比を保持）
        QSize labelSize = qrCodeLabel->size();
        // マージンを考慮してサイズを少し小さくする
        labelSize = DevTools::Ui::previewContentSize(labelSize);

        QPixmap const pixmap = QPixmap::fromImage(image);
        // アスペクト比を保持しながら、ラベルサイズに収まるようにスケーリング
        QPixmap const scaledPixmap =
            pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        qrCodeLabel->setPixmap(scaledPixmap);
        qrCodeLabel->show();

        // Enable output buttons
        copyButton->setEnabled(true);
        saveButton->setEnabled(true);

    } catch (const std::exception &e) {
        // QRコード生成時のエラーをキャッチ
        QMessageBox::critical(
            this, tr("Error"),
            tr("Failed to generate QR code:\n%1\n\n"
               "Please try reducing the amount of data or simplifying the content.")
                .arg(e.what()));

        // エラー時はボタンを無効化
        copyButton->setEnabled(false);
        saveButton->setEnabled(false);
    }
}

void QRCodeGenerationGUI::onClearClicked()
{
    clearAllParameters();
    qrCodeLabel->setText(tr("QR Code will appear here"));
    copyButton->setEnabled(false);
    saveButton->setEnabled(false);

    // 現在のQRコード画像をクリア
    currentQRImage = QImage();
}

void QRCodeGenerationGUI::onCopyClicked()
{
    // 画像が生成されているかチェック
    if (currentQRImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No QR code has been generated."));
        return;
    }

    // QRコード画像をクリップボードにコピー
    QApplication::clipboard()->setImage(currentQRImage);
    QMessageBox::information(this, tr("Copied"), tr("Copied the QR code image to the clipboard."));
}

void QRCodeGenerationGUI::onSaveClicked()
{
    // 画像が生成されているかチェック
    if (currentQRImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No QR code has been generated."));
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
                                     tr("Saved the QR code image: %1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to save the QR code image."));
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
    QSize labelSize = qrCodeLabel->size();
    // マージンを考慮してサイズを少し小さくする
    labelSize = DevTools::Ui::previewContentSize(labelSize);

    QPixmap const pixmap = QPixmap::fromImage(currentQRImage);
    // アスペクト比を保持しながら、ラベルサイズに収まるようにスケーリング
    QPixmap const scaledPixmap =
        pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    qrCodeLabel->setPixmap(scaledPixmap);
}
