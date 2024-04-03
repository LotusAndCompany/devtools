#include <QScopedPointer>
#include <QTextStream>
#include <QtTest>

#define _TEST_CommonException
#include "core/exception/common_exception.h"
#undef _TEST_CommonException

namespace Test {
class TestCommonException : public QObject
{
    Q_OBJECT

private slots:
    // Test cases:
    void test_defaultConstructor();
    void test_messageConstructor();
    void test_clone();
    void test_raise();
    void test_outputOperator();
};

void TestCommonException::test_defaultConstructor()
{
    CommonException e;
    // デフォルトコンストラクタの場合、空文字列が設定されること
    QVERIFY(e.message == "");
}

void TestCommonException::test_messageConstructor()
{
    const QString msg = "Custom message";
    CommonException e(msg);
    // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
    QVERIFY(e.message == msg);
}

void TestCommonException::test_clone()
{
    const QString msg = "Custom message";
    CommonException e(msg);
    // QVERIFYはreturn;する可能性があるため、QScopedPointerに入れる
    const QScopedPointer<QException> copied(e.clone());

    // clone()を呼び出しても元のインスタンスが変わらないこと
    QVERIFY(e.message == msg);
    // clone()で返されたインスタンスのメッセージが元のインスタンスと変わらないこと
    QVERIFY(static_cast<CommonException *>(copied.get())->message == msg);
    // clone()で返されたインスタンスが元のインスタンスとは異なること
    QVERIFY(&e != copied.get());
}

void TestCommonException::test_raise()
{
    const QString msg = "Custom message";
    CommonException src(msg);

    // try-catchのスコープより寿命が長い例外インスタンスを投げる
    try {
        src.raise();
    } catch (CommonException &e) {
        // srcが変更されていないこと
        QVERIFY(src.message == msg);

        // eとsrcとの内容が同じであること
        QVERIFY(e.message == msg);

        return;
    }

    QFAIL("raise() did not throw CommonException");
}

void TestCommonException::test_outputOperator()
{
    QString out;
    QTextStream stream(&out);

    const QString msg = "Custom message";
    CommonException e(msg);

    stream << e;

    QVERIFY(stream.readAll() == e.message);
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestCommonException)

#include "test_common_exception.moc"
