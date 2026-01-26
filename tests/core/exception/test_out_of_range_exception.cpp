#include "tests/test_util.h"

#include <QScopedPointer>
#include <QtTest>

#define _TEST_OutOfRangeException
#include "core/exception/out_of_range_exception.h"
#undef _TEST_OutOfRangeException

namespace Test {
class TestOutOfRangeException : public QObject
{
    Q_OBJECT

    RandomData rd;
    static const int length = 64;

private slots:
    // Test cases:
    static void test_defaultConstructor();
    void test_messageConstructor();
    static void test_actualMaxConstructor();
    static void test_actualMinMaxConstructor();
    void test_clone();
    void test_raise();
};

void TestOutOfRangeException::test_defaultConstructor()
{
    OutOfRangeException<int> const e;
    // デフォルトコンストラクタの場合、空文字列が設定されること
    QCOMPARE_EQ(e.message, "");
}

void TestOutOfRangeException::test_messageConstructor()
{
    const QString msg = rd.nextQString(length);
    OutOfRangeException<int> const e(msg);
    // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
    QCOMPARE_EQ(e.message, msg);
}

void TestOutOfRangeException::test_actualMaxConstructor()
{
    {
        OutOfRangeException const e(0x10, 10);
        // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
        QCOMPARE_EQ(e.message, "[OutOfRangeException] actual: 16, max: 10");
    }

    {
        OutOfRangeException const e(6.28318530718, 1.0);
        // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
        QCOMPARE_EQ(e.message, "[OutOfRangeException] actual: 6.28319, max: 1");
    }
}

void TestOutOfRangeException::test_actualMinMaxConstructor()
{
    {
        OutOfRangeException const e(-1, 0, 10);
        // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
        QCOMPARE_EQ(e.message, "[OutOfRangeException] actual: -1, min: 0, max: 10");
    }

    {
        OutOfRangeException const e(-0.5, 0.0, 1.0);
        // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
        QCOMPARE_EQ(e.message, "[OutOfRangeException] actual: -0.5, min: 0, max: 1");
    }
}

void TestOutOfRangeException::test_clone()
{
    const QString msg = rd.nextQString(length);
    OutOfRangeException<int> e(msg);
    // QVERIFYはreturn;する可能性があるため、QScopedPointerに入れる
    const QScopedPointer<QException> copied(e.clone());

    // clone()を呼び出しても元のインスタンスが変わらないこと
    QCOMPARE_EQ(e.message, msg);
    // clone()で返されたインスタンスのメッセージが元のインスタンスと変わらないこと
    QCOMPARE_EQ(dynamic_cast<OutOfRangeException<int> *>(copied.get())->message, msg);
    // clone()で返されたインスタンスが元のインスタンスとは異なること
    QCOMPARE_NE(&e, copied.get());
}

void TestOutOfRangeException::test_raise()
{
    const QString msg = rd.nextQString(length);
    OutOfRangeException<int> const src(msg);

    // try-catchのスコープより寿命が長い例外インスタンスを投げる
    try {
        src.raise();
    } catch (OutOfRangeException<int> &e) {
        // srcが変更されていないこと
        QCOMPARE_EQ(src.message, msg);

        // eとsrcとの内容が同じであること
        QCOMPARE_EQ(e.message, msg);

        return;
    }

    QFAIL("raise() did not throw OutOfRangeException");
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestOutOfRangeException)

#include "test_out_of_range_exception.moc"
