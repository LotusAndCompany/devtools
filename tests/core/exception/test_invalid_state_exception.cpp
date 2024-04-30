#include <QScopedPointer>
#include <QtTest>
#include "tests/test_util.h"

#define _TEST_InvalidStateException
#include "core/exception/invalid_state_exception.h"
#undef _TEST_InvalidStateException

namespace Test {
class TestInvalidStateException : public QObject
{
    Q_OBJECT

    RandomData rd;
    static const int length = 64;

private slots:
    // Test cases:
    void test_defaultConstructor();
    void test_messageConstructor();
    void test_actualExpectedConstructor();
    void test_clone();
    void test_raise();
};

void TestInvalidStateException::test_defaultConstructor()
{
    InvalidStateException e;
    // デフォルトコンストラクタの場合、空文字列が設定されること
    QCOMPARE_EQ(e.message, "");
}

void TestInvalidStateException::test_messageConstructor()
{
    const QString msg = rd.nextQString(length);
    InvalidStateException e(msg);
    // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
    QCOMPARE_EQ(e.message, msg);
}

void TestInvalidStateException::test_actualExpectedConstructor()
{
    InvalidStateException e("_actual_", "_expected_");
    // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
    QCOMPARE_EQ(e.message, "[InvalidStateException] actual: _actual_, expected: _expected_");
}

void TestInvalidStateException::test_clone()
{
    const QString msg = rd.nextQString(length);
    InvalidStateException e(msg);
    // QVERIFYはreturn;する可能性があるため、QScopedPointerに入れる
    const QScopedPointer<QException> copied(e.clone());

    // clone()を呼び出しても元のインスタンスが変わらないこと
    QCOMPARE_EQ(e.message, msg);
    // clone()で返されたインスタンスのメッセージが元のインスタンスと変わらないこと
    QCOMPARE_EQ(static_cast<InvalidStateException *>(copied.get())->message, msg);
    // clone()で返されたインスタンスが元のインスタンスとは異なること
    QCOMPARE_NE(&e, copied.get());
}

void TestInvalidStateException::test_raise()
{
    const QString msg = rd.nextQString(length);
    InvalidStateException src(msg);

    // try-catchのスコープより寿命が長い例外インスタンスを投げる
    try {
        src.raise();
    } catch (InvalidStateException &e) {
        // srcが変更されていないこと
        QCOMPARE_EQ(src.message, msg);

        // eとsrcとの内容が同じであること
        QCOMPARE_EQ(e.message, msg);

        return;
    }

    QFAIL("raise() did not throw InvalidStateException");
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestInvalidStateException)

#include "test_invalid_state_exception.moc"
