#include <QtTest>
#include "tests/test_util.h"

#define _TEST_BasicEmitter
#include "core/data_conversion/emitter/basic_emitter.h"
#undef _TEST_BasicEmitter

#include <QObject>

namespace Test {
class TestBasicEmitter : public QObject
{
    Q_OBJECT

private slots:
    void test_replace4spaceIndentation();
};

void TestBasicEmitter::test_replace4spaceIndentation()
{
    const QString src4spaces = QString("a    \n    bb    \n        ccc    \n        dddd    \neeeee    ");
    BasicEmitter emitter;
    // 4スペース→2スペースの変換ができること
    const QString src2spaces = QString("a    \n  bb    \n    ccc    \n    dddd    \neeeee    \n");
    QCOMPARE_EQ(emitter.replace4spaceIndentation(src4spaces, "  "), src2spaces);
    // 4スペース→タブの変換の変換ができること
    const QString srcTabs = QString("a    \n\tbb    \n\t\tccc    \n\t\tdddd    \neeeee    \n");
    QCOMPARE_EQ(emitter.replace4spaceIndentation(src4spaces, "\t"), srcTabs);

    // 空文字列を扱えること
    QVERIFY_THROWS_NO_EXCEPTION(emitter.replace4spaceIndentation("", "  "));
    // 空文字列+改行を扱えること
    QVERIFY_THROWS_NO_EXCEPTION(emitter.replace4spaceIndentation("\n", "  "));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestBasicEmitter)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestBasicEmitter)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_MAIN(Test::TestBasicEmitter)

#include "test_basic_emitter.moc"
