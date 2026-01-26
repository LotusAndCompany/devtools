#include <QtTest>

#define _TEST_BasicParser
#include "core/data_conversion/parser/basic_parser.h"
#undef _TEST_BasicParser

#include <QObject>

#include <core/exception/invalid_argument_exception.h>

namespace Test {
class TestBasicParser : public QObject
{
    Q_OBJECT

private slots:
    // test cases
    static void test_ParseResult();
    static void test_operator_bool();
};

void TestBasicParser::test_ParseResult()
{
    const BasicParser::ParseResult result;

    // successの初期値がfalseであること
    QCOMPARE_EQ(result.success, false);

    // dataの初期値がQVariantの初期値であること
    QCOMPARE_EQ(result.data, QVariant());

    // errorsの初期値が空の配列であること
    QVERIFY(result.errors.isEmpty());

    // extrasの初期値が空の連想配列であること
    QVERIFY(result.extras.empty());
}

void TestBasicParser::test_operator_bool()
{
    BasicParser::ParseResult result;

    // success=falseの時はfalseであること
    QVERIFY(!result);

    // success=trueの時はtrueであること
    result.success = true;
    QVERIFY(result);
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestBasicParser)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
// QTEST_GUILESS_MAIN(Test::TestbasicParser)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
// QTEST_MAIN(Test::TestbasicParser)

#include "test_basic_parser.moc"
