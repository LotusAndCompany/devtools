#include "features/command/gui/command.h"

#include <QComboBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QtTest>

namespace Test {
class TestCommand : public QObject
{
    Q_OBJECT

private slots:
    void generatesCommand();
};

void TestCommand::generatesCommand()
{
    Command command;

    auto *const categoryList = command.findChild<QComboBox *>(QStringLiteral("categoryList"));
    auto *const functionsList = command.findChild<QComboBox *>(QStringLiteral("functionsList"));
    auto *const optionList = command.findChild<QComboBox *>(QStringLiteral("optionList"));
    auto *const generateButton = command.findChild<QPushButton *>(QStringLiteral("generateButton"));
    auto *const textBrowser = command.findChild<QPlainTextEdit *>(QStringLiteral("textBrowser"));

    QVERIFY(categoryList != nullptr);
    QVERIFY(functionsList != nullptr);
    QVERIFY(optionList != nullptr);
    QVERIFY(generateButton != nullptr);
    QVERIFY(textBrowser != nullptr);

    categoryList->setCurrentIndex(1);
    functionsList->setCurrentIndex(2);
    optionList->setCurrentIndex(0);
    generateButton->click();

    QCOMPARE(textBrowser->toPlainText(), QStringLiteral("git log"));
}
} // namespace Test

QTEST_MAIN(Test::TestCommand)

#include "test_command.moc"
