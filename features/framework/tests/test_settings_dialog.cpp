#include "features/framework/gui/menubar/settings_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

namespace {
QCheckBox *checkBoxWithText(QWidget &parent, const QString &text)
{
    for (QCheckBox *checkBox : parent.findChildren<QCheckBox *>()) {
        if (checkBox->text() == text) {
            return checkBox;
        }
    }
    return nullptr;
}

void configureSettingsStorage(const QString &path)
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, path);
    QCoreApplication::setOrganizationName(QStringLiteral("DevToolsSettingsTest"));
    QCoreApplication::setApplicationName(QStringLiteral("test_settings_dialog"));
}
} // namespace

class TestSettingsDialog : public QObject
{
    Q_OBJECT

private slots:
    void loadsDefaultsAndAppliesPreferences();
};

void TestSettingsDialog::loadsDefaultsAndAppliesPreferences()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    configureSettingsStorage(tempDir.path());
    {
        QSettings settings;
        settings.clear();
        settings.sync();
    }

    SettingsDialog dialog;
    auto *const language = dialog.findChild<QComboBox *>();
    QVERIFY(language != nullptr);
    QCOMPARE(language->currentData().toString(), QStringLiteral("en"));

    auto *const showSidebar = checkBoxWithText(dialog, QStringLiteral("Show sidebar on startup"));
    auto *const showLastTool =
        checkBoxWithText(dialog, QStringLiteral("Open last used tool on startup"));
    auto *const alwaysOnTop = checkBoxWithText(dialog, QStringLiteral("Always on top"));
    auto *const rememberSize = checkBoxWithText(dialog, QStringLiteral("Remember window size"));
    auto *const rememberPosition =
        checkBoxWithText(dialog, QStringLiteral("Remember window position"));
    QVERIFY(showSidebar != nullptr);
    QVERIFY(showLastTool != nullptr);
    QVERIFY(alwaysOnTop != nullptr);
    QVERIFY(rememberSize != nullptr);
    QVERIFY(rememberPosition != nullptr);
    QVERIFY(showSidebar->isChecked());
    QVERIFY(!showLastTool->isChecked());
    QVERIFY(!alwaysOnTop->isChecked());
    QVERIFY(rememberSize->isChecked());
    QVERIFY(rememberPosition->isChecked());

    QSignalSpy languageChangedSpy(&dialog, &SettingsDialog::languageChanged);
    language->setCurrentIndex(language->findData(QStringLiteral("ja_JP")));
    showSidebar->setChecked(false);
    showLastTool->setChecked(true);
    alwaysOnTop->setChecked(true);
    rememberSize->setChecked(false);
    rememberPosition->setChecked(false);

    auto *const buttonBox = dialog.findChild<QDialogButtonBox *>();
    QVERIFY(buttonBox != nullptr);
    auto *const applyButton = buttonBox->button(QDialogButtonBox::Apply);
    QVERIFY(applyButton != nullptr);
    applyButton->click();

    QCOMPARE(languageChangedSpy.size(), 1);
    QCOMPARE(languageChangedSpy.takeFirst().at(0).toString(), QStringLiteral("ja_JP"));
    QSettings persisted;
    QCOMPARE(persisted.value(QStringLiteral("language")).toString(), QStringLiteral("ja_JP"));
    QVERIFY(!persisted.value(QStringLiteral("general/showSidemenuOnStartup")).toBool());
    QVERIFY(persisted.value(QStringLiteral("general/showLastToolOnStartup")).toBool());
    QVERIFY(persisted.value(QStringLiteral("window/alwaysOnTop")).toBool());
    QVERIFY(!persisted.value(QStringLiteral("window/rememberSize")).toBool());
    QVERIFY(!persisted.value(QStringLiteral("window/rememberPosition")).toBool());
}

QTEST_MAIN(TestSettingsDialog)
#include "test_settings_dialog.moc"
