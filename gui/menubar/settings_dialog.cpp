#include "settings_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QShowEvent>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QTabWidget>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setModal(true);
    resize(480, 360);

    auto *rootLayout = new QVBoxLayout(this);

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

    auto *layout = new QVBoxLayout(general_tab);
    layout->setSpacing(10);
    layout->setContentsMargins(8, 8, 8, 8);

    auto *languageLayout = new QHBoxLayout;
    languageLayout->setSpacing(6);
    languageLayout->setContentsMargins(0, 0, 0, 0);

    language_label = new QLabel(general_tab);
    language_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    language_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    languageLayout->addWidget(language_label);

    language_combo_box = new QComboBox(general_tab);
    language_combo_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    language_combo_box->setMinimumSize(96, 0);
    language_combo_box->setMaximumWidth(96);
    language_combo_box->addItem(tr("English"), QStringLiteral("en"));
    language_combo_box->addItem(tr("Japanese"), QStringLiteral("ja_JP"));
    languageLayout->addWidget(language_combo_box);

    languageLayout->addSpacerItem(
        new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->addLayout(languageLayout);

    show_sidebar_on_startup_check_box = new QCheckBox(general_tab);
    layout->addWidget(show_sidebar_on_startup_check_box);

    show_last_tool_on_startup_check_box = new QCheckBox(general_tab);
    layout->addWidget(show_last_tool_on_startup_check_box);

    layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // 翻訳可能なテキストを設定
    language_label->setText(tr("Language:"));
    show_sidebar_on_startup_check_box->setText(tr("Show sidebar on startup"));
    show_last_tool_on_startup_check_box->setText(tr("Open last used tool on startup"));
}

void SettingsDialog::buildWindowTab()
{
    window_tab = new QWidget(tab_widget);
    auto *layout = new QVBoxLayout(window_tab);

    window_behavior_group_box = new QGroupBox(window_tab);
    auto *groupLayout = new QVBoxLayout(window_behavior_group_box);

    always_on_top_check_box = new QCheckBox(window_behavior_group_box);
    groupLayout->addWidget(always_on_top_check_box);

    remember_window_size_check_box = new QCheckBox(window_behavior_group_box);
    groupLayout->addWidget(remember_window_size_check_box);

    remember_window_position_check_box = new QCheckBox(window_behavior_group_box);
    groupLayout->addWidget(remember_window_position_check_box);

    layout->addWidget(window_behavior_group_box);
    layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

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

    QSettings settings;

    // 言語設定
    QString const language = settings.value("language", "en").toString();
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
    const QString showSidebarKey = QStringLiteral("general/showSidemenuOnStartup");
    if (!settings.contains(showSidebarKey)) {
        settings.setValue(showSidebarKey, true);
    }
    bool const showSidebarOnStartup = settings.value(showSidebarKey, true).toBool();
    show_sidebar_on_startup_check_box->setChecked(showSidebarOnStartup);

    const QString showLastToolKey = QStringLiteral("general/showLastToolOnStartup");
    if (!settings.contains(showLastToolKey)) {
        settings.setValue(showLastToolKey, false);
    }
    bool const showLastToolOnStartup = settings.value(showLastToolKey, false).toBool();
    show_last_tool_on_startup_check_box->setChecked(showLastToolOnStartup);

    // ウィンドウ設定
    bool alwaysOnTop = false;
    if (parentWidget() != nullptr) {
        alwaysOnTop = ((parentWidget()->windowFlags() & Qt::WindowStaysOnTopHint) != 0U);
    }
    if (!alwaysOnTop) {
        alwaysOnTop = settings.value("window/alwaysOnTop", false).toBool();
    }
    always_on_top_check_box->setChecked(alwaysOnTop);

    bool const rememberWindowSize = settings.value("window/rememberSize", true).toBool();
    remember_window_size_check_box->setChecked(rememberWindowSize);

    bool const rememberWindowPosition = settings.value("window/rememberPosition", true).toBool();
    remember_window_position_check_box->setChecked(rememberWindowPosition);

    qDebug() << "SettingsDialog::loadSettings() completed";
}

void SettingsDialog::saveSettings()
{
    QSettings settings;

    // 言語設定
    QString const language = language_combo_box->currentData().toString();
    QString const currentLanguage = settings.value("language", "en").toString();
    qDebug() << "SettingsDialog::saveSettings - Language change:" << currentLanguage << "->"
             << language;
    settings.setValue("language", language);
    if (language != currentLanguage) {
        qDebug() << "Emitting languageChanged signal for:" << language;
        emit languageChanged(language);
    }

    // サイドバー設定
    const QString showSidebarKey = QStringLiteral("general/showSidemenuOnStartup");
    bool const showSidebarOnStartup = show_sidebar_on_startup_check_box->isChecked();
    settings.setValue(showSidebarKey, showSidebarOnStartup);

    const QString showLastToolKey = QStringLiteral("general/showLastToolOnStartup");
    bool const showLastToolOnStartup = show_last_tool_on_startup_check_box->isChecked();
    settings.setValue(showLastToolKey, showLastToolOnStartup);

    // ウィンドウ設定
    bool const alwaysOnTop = always_on_top_check_box->isChecked();
    settings.setValue("window/alwaysOnTop", alwaysOnTop);
    settings.setValue("window/rememberSize", remember_window_size_check_box->isChecked());
    settings.setValue("window/rememberPosition", remember_window_position_check_box->isChecked());

    // Always on topを即座に適用
    if (parentWidget() != nullptr) {
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
