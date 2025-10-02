#include "settings_dialog.h"
#include "ui_settings_dialog.h"
#include "../gui_application.h"

#include <QSettings>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QShowEvent>
#include <QIcon>
#include <QApplication>
#include <QEvent>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // コンボボックスのデータを手動で設定
    qDebug() << "Setting up combo boxes...";
    
    // 言語コンボボックス
    ui->languageComboBox->clear();
    ui->languageComboBox->addItem("日本語", "ja_JP");
    ui->languageComboBox->addItem("English", "en");
    
    qDebug() << "Combo boxes setup completed";

    // ボタンの接続
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::onRejected);
    
    // 適用ボタンがある場合の接続
    if (auto *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply)) {
        connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    }

    // 設定を読み込み
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
void SettingsDialog::showEvent(QShowEvent *event)
{
    qDebug() << "SettingsDialog::showEvent() called";
    QDialog::showEvent(event);
    // 表示時に設定を再読み込み
    loadSettings();
}

void SettingsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        qDebug() << "SettingsDialog: Language change detected, retranslating UI";
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void SettingsDialog::loadSettings()
{
    qDebug() << "SettingsDialog::loadSettings() called";
    
    // コンボボックスのデータが空の場合、強制的に設定
    if (ui->languageComboBox->itemData(0).toString().isEmpty()) {
        qDebug() << "Language combo data is empty, setting manually";
        ui->languageComboBox->setItemData(0, "ja_JP");
        ui->languageComboBox->setItemData(1, "en");
    }
    
    
    QSettings settings;

    // 言語設定
    QString language = settings.value("language", "ja_JP").toString();
    qDebug() << "Loading language setting:" << language;
    
    // コンボボックスのアイテム数とデータを確認
    qDebug() << "Language combo box has" << ui->languageComboBox->count() << "items";
    for (int i = 0; i < ui->languageComboBox->count(); ++i) {
        qDebug() << "  Item" << i << ":" << ui->languageComboBox->itemText(i) 
                 << "data:" << ui->languageComboBox->itemData(i).toString();
    }
    
    int languageIndex = ui->languageComboBox->findData(language);
    qDebug() << "Found language index:" << languageIndex;
    if (languageIndex >= 0) {
        ui->languageComboBox->setCurrentIndex(languageIndex);
    } else {
        // デフォルトを設定
        ui->languageComboBox->setCurrentIndex(0);
        qDebug() << "Language not found, set to index 0";
    }


    // ウィンドウ設定
    // MainWindowの現在の状態を反映するため、親ウィンドウから取得
    bool alwaysOnTop = false;
    if (parentWidget()) {
        alwaysOnTop = parentWidget()->windowFlags() & Qt::WindowStaysOnTopHint;
    }
    if (!alwaysOnTop) {
        // 親ウィンドウから取得できない場合は設定から読む
        alwaysOnTop = settings.value("window/alwaysOnTop", false).toBool();
    }
    ui->alwaysOnTopCheckBox->setChecked(alwaysOnTop);

    bool rememberWindowSize = settings.value("window/rememberSize", true).toBool();
    ui->rememberWindowSizeCheckBox->setChecked(rememberWindowSize);

    bool rememberWindowPosition = settings.value("window/rememberPosition", true).toBool();
    ui->rememberWindowPositionCheckBox->setChecked(rememberWindowPosition);
    
    qDebug() << "SettingsDialog::loadSettings() completed";
}

void SettingsDialog::saveSettings()
{
    QSettings settings;

    // 言語設定
    QString language = ui->languageComboBox->currentData().toString();
    QString currentLanguage = settings.value("language", "ja_JP").toString();
    qDebug() << "SettingsDialog::saveSettings - Language change:" << currentLanguage << "->" << language;
    settings.setValue("language", language);
    if (language != currentLanguage) {
        qDebug() << "Emitting languageChanged signal for:" << language;
        emit languageChanged(language);
    }


    // ウィンドウ設定
    bool alwaysOnTop = ui->alwaysOnTopCheckBox->isChecked();
    settings.setValue("window/alwaysOnTop", alwaysOnTop);
    settings.setValue("window/rememberSize", ui->rememberWindowSizeCheckBox->isChecked());
    settings.setValue("window/rememberPosition", ui->rememberWindowPositionCheckBox->isChecked());
    
    // Always on topを即座に適用
    if (parentWidget()) {
        parentWidget()->setWindowFlag(Qt::WindowStaysOnTopHint, alwaysOnTop);
        parentWidget()->show();
    }

    settings.sync();
    qDebug() << "SettingsDialog::saveSettings completed";
}

void SettingsDialog::onAccepted()
{
    qDebug() << "SettingsDialog::onAccepted() called";
    saveSettings();
    accept();
}

void SettingsDialog::onRejected()
{
    reject();
}

void SettingsDialog::onApplyClicked()
{
    qDebug() << "SettingsDialog::onApplyClicked() called";
    saveSettings();
}

