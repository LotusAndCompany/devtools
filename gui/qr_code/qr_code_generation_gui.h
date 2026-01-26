#ifndef QR_CODE_GENERATION_GUI_H
#define QR_CODE_GENERATION_GUI_H

#include "gui/gui_tool.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <QWidget>

namespace Ui {
class QRCodeGenerationGUI;
}

class QTextEdit;

class QRCodeGenerationGUI : public GuiTool
{
    Q_OBJECT
    Q_DISABLE_COPY(QRCodeGenerationGUI)

public:
    explicit QRCodeGenerationGUI(QWidget *parent = nullptr);
    ~QRCodeGenerationGUI() override;

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
    Ui::QRCodeGenerationGUI *ui;

    enum QRCodeType { Text, Url, Email, Phone, Sms, Wifi, Contact, Calendar, Geo };

    void initializeCategories();
    void setupParameterWidgets();
    void clearAllParameters();
    QString generateQRCodeContent();

    // Parameter widget for text type
    QWidget *createTextWidget();
    QWidget *createUrlWidget();
    QWidget *createEmailWidget();
    QWidget *createPhoneWidget();
    QWidget *createSmsWidget();
    QWidget *createWifiWidget();
    QWidget *createContactWidget();
    QWidget *createCalendarWidget();
    QWidget *createGeoWidget();

    QStackedWidget *parameterStack{nullptr};
    QRCodeType currentType{Text};

    // Widget references for easy access
    QMap<QString, QWidget *> parameterWidgets;

    // エラーラベル用のマップ
    QMap<QString, QLabel *> errorLabels;

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
