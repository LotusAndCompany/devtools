#include <QScopedPointer>
#include <QtTest>
#include "tests/test_util.h"

#define _TEST_InvalidArgumentException
#include "core/exception/invalid_argument_exception.h"
#undef _TEST_InvalidArgumentException

namespace Test {
class TestInvalidArgumentException : public QObject
{
    Q_OBJECT

    RandomData rd;
    static const int length = 64;

private slots:
    // Test cases:
    void test_defaultConstructor();
    void test_messageConstructor();
    void test_argumentReasonConstructor();
    void test_clone();
    void test_raise();
};

void TestInvalidArgumentException::test_defaultConstructor()
{
    InvalidArgumentException<int> e;
    // デフォルトコンストラクタの場合、空文字列が設定されること
    QCOMPARE_EQ(e.message, "");
}

void TestInvalidArgumentException::test_messageConstructor()
{
    const QString msg = rd.nextQString(length);
    InvalidArgumentException<int> e(msg);
    // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
    QCOMPARE_EQ(e.message, msg);
}

void TestInvalidArgumentException::test_argumentReasonConstructor()
{
    {
        const QString reason_int = "int_why?";
        InvalidArgumentException e_int(0xf, reason_int);
        // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
        QCOMPARE_EQ(e_int.message, "[InvalidArgumentException] given: 15, reason: int_why?");
    }

    {
        const QString reason_double = "double_why?";
        InvalidArgumentException e_double(2.718281828459045235360287471352, reason_double);
        // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
        // NOTE: 浮動小数点型をQStringにする時のデフォルトの精度は6桁
        QCOMPARE_EQ(e_double.message,
                    "[InvalidArgumentException] given: 2.71828, reason: double_why?");
    }

    {
        const QString reason_str = "str_why?";
        InvalidArgumentException e_str(QString("qwerty"), reason_str);
        // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
        QCOMPARE_EQ(e_str.message, "[InvalidArgumentException] given: qwerty, reason: str_why?");
    }
}

void TestInvalidArgumentException::test_clone()
{
    const QString msg = rd.nextQString(length);
    InvalidArgumentException<int> e(msg);
    // QVERIFYはreturn;する可能性があるため、QScopedPointerに入れる
    const QScopedPointer<QException> copied(e.clone());

    // clone()を呼び出しても元のインスタンスが変わらないこと
    QCOMPARE_EQ(e.message, msg);
    // clone()で返されたインスタンスのメッセージが元のインスタンスと変わらないこと
    QCOMPARE_EQ(static_cast<InvalidArgumentException<int> *>(copied.get())->message, msg);
    // clone()で返されたインスタンスが元のインスタンスとは異なること
    QCOMPARE_NE(&e, copied.get());
}

void TestInvalidArgumentException::test_raise()
{
    const QString msg = rd.nextQString(length);
    InvalidArgumentException<int> src(msg);

    // try-catchのスコープより寿命が長い例外インスタンスを投げる
    try {
        src.raise();
    } catch (InvalidArgumentException<int> &e) {
        // srcが変更されていないこと
        QCOMPARE_EQ(src.message, msg);

        // eとsrcとの内容が同じであること
        QCOMPARE_EQ(e.message, msg);

        return;
    }

    QFAIL("raise() did not throw InvalidArgumentException<int>");
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestInvalidArgumentException)

#include "test_invalid_argument_exception.moc"
