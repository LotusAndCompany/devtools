#include <QScopedPointer>
#include <QtTest>

#define _TEST_UnderDevelopmentException
#include "core/exception/under_development_exception.h"
#undef _TEST_UnderDevelopmentException

namespace Test {
class TestUnderDevelopmentException : public QObject
{
    Q_OBJECT

private slots:
    static void test_defaultConstructor();
    static void test_messageConstructor();
    static void test_clone();
    static void test_raise();
};

void TestUnderDevelopmentException::test_defaultConstructor()
{
    UnderDevelopmentException const e;
    // デフォルトコンストラクタの場合、"[UnderDevelopmentException] Under
    // development..."が設定されること
    QCOMPARE_EQ(e.message, "[UnderDevelopmentException] Under development...");
}

void TestUnderDevelopmentException::test_messageConstructor()
{
    const QString msg = "Custom message";
    UnderDevelopmentException const e(msg);
    // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
    QCOMPARE_EQ(e.message, msg);
}

void TestUnderDevelopmentException::test_clone()
{
    const QString msg = "Custom message";
    UnderDevelopmentException e(msg);
    // QVERIFYはreturn;する可能性があるため、QScopedPointerに入れる
    const QScopedPointer<QException> copied(e.clone());

    // clone()を呼び出しても元のインスタンスが変わらないこと
    QCOMPARE_EQ(e.message, msg);
    // clone()で返されたインスタンスのメッセージが元のインスタンスと変わらないこと
    QCOMPARE_EQ(dynamic_cast<UnderDevelopmentException *>(copied.get())->message, msg);
    // clone()で返された物が元のインスタンスとは異なること
    QCOMPARE_NE(&e, copied.get());
}

void TestUnderDevelopmentException::test_raise()
{
    const QString msg = "Custom message";
    UnderDevelopmentException const src(msg);

    // try-catchのスコープより寿命が長い例外インスタンスを投げる
    try {
        src.raise();
    } catch (UnderDevelopmentException &e) {
        // srcが変更されていないこと
        QCOMPARE_EQ(src.message, msg);

        // eとsrcとの内容が同じであること
        QCOMPARE_EQ(e.message, msg);

        return;
    }

    QFAIL("raise() did not throw UnderDevelopmentException");
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestUnderDevelopmentException)

#include "test_under_development_exception.moc"
