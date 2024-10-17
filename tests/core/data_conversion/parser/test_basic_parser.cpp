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
    void test_validatePointer();
};

void TestBasicParser::test_validatePointer()
{
    // nullptr以外が渡された時は何もしないこと
    char *const byte = new char;
    QVERIFY_THROWS_NO_EXCEPTION(BasicParser::validatePointer(byte));
    delete byte;

    // nullptrが渡された時は例外を投げること
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<nullptr_t>,
                             BasicParser::validatePointer(nullptr));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestBasicParser)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestbasicParser)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_MAIN(Test::TestbasicParser)

#include "test_basic_parser.moc"
