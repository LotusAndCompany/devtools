#ifndef QR_CODE_GENERATION_GUI_H
#define QR_CODE_GENERATION_GUI_H

#include <QWidget>
#include <QMap>
#include <QStackedWidget>
#include "gui/gui_tool.h"

namespace Ui {
class QRCodeGenerationGUI;
}

class QLineEdit;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QDateTimeEdit;

class QRCodeGenerationGUI : public GuiTool
{
    Q_OBJECT

public:
    explicit QRCodeGenerationGUI(QWidget *parent = nullptr);
    ~QRCodeGenerationGUI();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onCategoryChanged(int index);
    void onGenerateClicked();
    void onClearClicked();
    void onCopyClicked();
    void onSaveClicked();
    void updateGenerateButtonState();

private:
    Ui::QRCodeGenerationGUI *ui;
    
    enum QRCodeType {
        Text,
        URL,
        Email,
        Phone,
        SMS,
        WiFi,
        Contact,
        CalendarEvent,
        GeoLocation
    };
    
    void initializeCategories();
    void setupParameterWidgets();
    void clearAllParameters();
    QString generateQRCodeContent();
    
    // Parameter widgets for different types
    QWidget* createTextWidget();
    QWidget* createURLWidget();
    QWidget* createEmailWidget();
    QWidget* createPhoneWidget();
    QWidget* createSMSWidget();
    QWidget* createWiFiWidget();
    QWidget* createContactWidget();
    QWidget* createCalendarWidget();
    QWidget* createGeoWidget();
    
    QStackedWidget* parameterStack;
    QRCodeType currentType;
    
    // Widget references for easy access
    QMap<QString, QWidget*> parameterWidgets;
};

#endif // QR_CODE_GENERATION_GUI_H