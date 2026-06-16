#ifndef QR_CODE_GENERATION_GUI_H
#define QR_CODE_GENERATION_GUI_H

#include "gui/components/status_label.h"
#include "gui/gui_tool.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <QWidget>

class QRCodeGenerationGUI : public GuiTool
{
    Q_OBJECT
    Q_DISABLE_COPY(QRCodeGenerationGUI)

public:
    explicit QRCodeGenerationGUI(QWidget *parent = nullptr);
    ~QRCodeGenerationGUI() override = default;
    QRCodeGenerationGUI(QRCodeGenerationGUI &&) = delete;
    QRCodeGenerationGUI &operator=(QRCodeGenerationGUI &&) = delete;

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onCategoryChanged(int index);
    void onGenerateClicked();
    void onClearClicked();
    void onCopyClicked();
    void onSaveClicked();
    void updateGenerateButtonState();

private:
    enum class QRCodeType : uint8_t { Text, Url, Email, Phone, Sms, Wifi, Contact, Calendar, Geo };

    void buildUi();
    void retranslateUi();
    void setupParameterWidgets();
    void clearAllParameters();
    QString generateQRCodeContent();

    // Parameter widget for each QR code type
    QWidget *createTextWidget();
    QWidget *createUrlWidget();
    QWidget *createEmailWidget();
    QWidget *createPhoneWidget();
    QWidget *createSmsWidget();
    QWidget *createWifiWidget();
    QWidget *createContactWidget();
    QWidget *createCalendarWidget();
    QWidget *createGeoWidget();

    // Top-level widgets (owned via Qt parent-child ownership)
    QGroupBox *categoryGroupBox{nullptr};
    QGroupBox *parametersGroupBox{nullptr};
    QGroupBox *outputGroupBox{nullptr};
    QComboBox *categoryComboBox{nullptr};
    QStackedWidget *parameterStackedWidget{nullptr};
    QPushButton *generateButton{nullptr};
    QPushButton *clearButton{nullptr};
    QPushButton *copyButton{nullptr};
    QPushButton *saveButton{nullptr};
    QLabel *qrCodeLabel{nullptr};
    QTextEdit *contentPreviewEdit{nullptr};

    QRCodeType currentType{QRCodeType::Text};

    // Widget references for easy access
    QMap<QString, QWidget *> parameterWidgets;

    // エラーラベル用のマップ
    QMap<QString, StatusLabel *> errorLabels;

    // バリデーションメソッド
    bool validateCurrentType();
    void showValidationError(const QString &fieldKey, const QString &message);
    void clearValidationErrors();
    static bool isValidPhoneNumber(const QString &phone);

    // 生成されたQRコード画像を保持
    QImage currentQRImage;

    // QRコード表示を更新
    void refreshQRCodeDisplay();
};

#endif // QR_CODE_GENERATION_GUI_H
