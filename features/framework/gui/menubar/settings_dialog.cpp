#include "settings_dialog.h"

#include "../design_system.h"
#include "features/framework/core/application_settings_service.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QTabWidget>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setModal(true);
    DevTools::Ui::configureDialog(this);

    auto *rootLayout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(rootLayout);

    tab_widget = new QTabWidget(this);
    buildGeneralTab();
    buildWindowTab();
    tab_widget->addTab(general_tab, tr("General"));
    tab_widget->addTab(window_tab, tr("Window"));
    tab_widget->setCurrentIndex(0);
    rootLayout->addWidget(tab_widget);

    button_box = new QDialogButtonBox(
        QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal,
        this);
    DevTools::Ui::configureDialogButtonBox(button_box);
    rootLayout->addWidget(button_box);

    qDebug() << "Setting up combo boxes...";
    qDebug() << "Combo boxes setup completed";

    connect(button_box, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(button_box, &QDialogButtonBox::rejected, this, &SettingsDialog::onRejected);
    if (auto *applyButton = button_box->button(QDialogButtonBox::Apply)) {
        connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    }

    loadSettings();
}

void SettingsDialog::buildGeneralTab()
{
    general_tab = new QWidget(tab_widget);

    auto *tabLayout = new QVBoxLayout(general_tab);
    DevTools::Ui::applyPanelLayout(tabLayout);

    general_group_box = DevTools::Ui::createPane(QString(), general_tab);
    DevTools::Ui::configureCompactPane(general_group_box);
    auto *groupLayout = new QVBoxLayout(general_group_box);
    DevTools::Ui::applyPanelLayout(groupLayout);

    auto *languageLayout = new QFormLayout;
    DevTools::Ui::configureInlineFormLayout(languageLayout);

    language_label = new QLabel(general_group_box);
    language_combo_box = new QComboBox(general_group_box);
    DevTools::Ui::configureComboBox(language_combo_box);
    language_combo_box->addItem(tr("English"), QStringLiteral("en"));
    language_combo_box->addItem(tr("Japanese"), QStringLiteral("ja_JP"));
    languageLayout->addRow(language_label, language_combo_box);
    groupLayout->addLayout(languageLayout);

    show_sidebar_on_startup_check_box = new QCheckBox(general_group_box);
    groupLayout->addWidget(show_sidebar_on_startup_check_box);

    show_last_tool_on_startup_check_box = new QCheckBox(general_group_box);
    groupLayout->addWidget(show_last_tool_on_startup_check_box);

    tabLayout->addWidget(general_group_box);
    tabLayout->addStretch();

    // 翻訳可能なテキストを設定
    general_group_box->setTitle(tr("General"));
    language_label->setText(tr("Language:"));
    show_sidebar_on_startup_check_box->setText(tr("Show sidebar on startup"));
    show_last_tool_on_startup_check_box->setText(tr("Open last used tool on startup"));
}

void SettingsDialog::buildWindowTab()
{
    window_tab = new QWidget(tab_widget);
    auto *layout = new QVBoxLayout(window_tab);
    DevTools::Ui::applyPanelLayout(layout);

    window_behavior_group_box = DevTools::Ui::createPane(QString(), window_tab);
    DevTools::Ui::configureCompactPane(window_behavior_group_box);
    auto *groupLayout = new QVBoxLayout(window_behavior_group_box);
    DevTools::Ui::applyPanelLayout(groupLayout);

    always_on_top_check_box = new QCheckBox(window_behavior_group_box);
    groupLayout->addWidget(always_on_top_check_box);

    remember_window_size_check_box = new QCheckBox(window_behavior_group_box);
    groupLayout->addWidget(remember_window_size_check_box);

    remember_window_position_check_box = new QCheckBox(window_behavior_group_box);
    groupLayout->addWidget(remember_window_position_check_box);

    layout->addWidget(window_behavior_group_box);
    layout->addStretch();

    // 翻訳可能なテキストを設定
    window_behavior_group_box->setTitle(tr("Window Behavior"));
    always_on_top_check_box->setText(tr("Always on top"));
    remember_window_size_check_box->setText(tr("Remember window size"));
    remember_window_position_check_box->setText(tr("Remember window position"));
}

void SettingsDialog::retranslateUi()
{
    setWindowTitle(tr("Settings"));
    tab_widget->setTabText(0, tr("General"));
    tab_widget->setTabText(1, tr("Window"));
    general_group_box->setTitle(tr("General"));
    language_label->setText(tr("Language:"));
    if (language_combo_box->count() >= 2) {
        language_combo_box->setItemText(0, tr("English"));
        language_combo_box->setItemText(1, tr("Japanese"));
    }
    show_sidebar_on_startup_check_box->setText(tr("Show sidebar on startup"));
    show_last_tool_on_startup_check_box->setText(tr("Open last used tool on startup"));
    window_behavior_group_box->setTitle(tr("Window Behavior"));
    always_on_top_check_box->setText(tr("Always on top"));
    remember_window_size_check_box->setText(tr("Remember window size"));
    remember_window_position_check_box->setText(tr("Remember window position"));
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
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void SettingsDialog::loadSettings()
{
    qDebug() << "SettingsDialog::loadSettings() called";

    // コンボボックスのデータが空の場合、強制的に設定
    if (language_combo_box->itemData(0).toString().isEmpty()) {
        qDebug() << "Language combo data is empty, setting manually";
        language_combo_box->setItemData(0, QStringLiteral("en"));
        language_combo_box->setItemData(1, QStringLiteral("ja_JP"));
    }

    const ApplicationSettings settings = ApplicationSettingsService().read();

    // 言語設定
    QString const language = settings.language;
    qDebug() << "Loading language setting:" << language;

    qDebug() << "Language combo box has" << language_combo_box->count() << "items";
    for (int i = 0; i < language_combo_box->count(); ++i) {
        qDebug() << "  Item" << i << ":" << language_combo_box->itemText(i)
                 << "data:" << language_combo_box->itemData(i).toString();
    }

    int const languageIndex = language_combo_box->findData(language);
    qDebug() << "Found language index:" << languageIndex;
    if (languageIndex >= 0) {
        language_combo_box->setCurrentIndex(languageIndex);
    } else {
        language_combo_box->setCurrentIndex(0);
        qDebug() << "Language not found, set to index 0";
    }

    // サイドバー設定
    show_sidebar_on_startup_check_box->setChecked(settings.showSidemenuOnStartup);
    show_last_tool_on_startup_check_box->setChecked(settings.showLastToolOnStartup);

    // ウィンドウ設定
    bool alwaysOnTop = settings.alwaysOnTop;
    if (parentWidget() != nullptr) {
        if ((parentWidget()->windowFlags() & Qt::WindowStaysOnTopHint) != 0U) {
            alwaysOnTop = true;
        }
    }
    always_on_top_check_box->setChecked(alwaysOnTop);

    remember_window_size_check_box->setChecked(settings.rememberWindowSize);
    remember_window_position_check_box->setChecked(settings.rememberWindowPosition);

    qDebug() << "SettingsDialog::loadSettings() completed";
}

void SettingsDialog::saveSettings()
{
    ApplicationSettingsService settings;
    ApplicationSettings values = settings.read();

    // 言語設定
    QString const language = language_combo_box->currentData().toString();
    QString const currentLanguage = values.language;
    qDebug() << "SettingsDialog::saveSettings - Language change:" << currentLanguage << "->"
             << language;
    values.language = language;
    if (language != currentLanguage) {
        qDebug() << "Emitting languageChanged signal for:" << language;
        emit languageChanged(language);
    }

    // サイドバー設定
    values.showSidemenuOnStartup = show_sidebar_on_startup_check_box->isChecked();
    values.showLastToolOnStartup = show_last_tool_on_startup_check_box->isChecked();

    // ウィンドウ設定
    bool const alwaysOnTop = always_on_top_check_box->isChecked();
    values.alwaysOnTop = alwaysOnTop;
    values.rememberWindowSize = remember_window_size_check_box->isChecked();
    values.rememberWindowPosition = remember_window_position_check_box->isChecked();

    // Always on topを即座に適用
    if (parentWidget() != nullptr) {
        parentWidget()->setWindowFlag(Qt::WindowStaysOnTopHint, alwaysOnTop);
        parentWidget()->show();
    }

    settings.savePreferences(values);
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
