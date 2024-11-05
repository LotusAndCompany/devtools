#include <QScopedPointer>
#include <QtTest>
#include "tests/test_util.h"

#define _TEST_CommonException
#include "core/exception/common_exception.h"
#undef _TEST_CommonException

namespace Test {
class TestCommonException : public QObject
{
    Q_OBJECT

    RandomData rd;
    static const int length = 64;

private slots:
    // Test cases:
    void test_defaultConstructor();
    void test_messageConstructor();
    void test_clone();
    void test_raise();
    void test_QDebugOutputOperator();
};

void TestCommonException::test_defaultConstructor()
{
    CommonException e;
    // デフォルトコンストラクタの場合、空文字列が設定されること
    QVERIFY(e.message == "");
}

void TestCommonException::test_messageConstructor()
{
    const QString msg = rd.nextQString(length);
    CommonException e(msg);
    // コンストラクタでメッセージを設定した場合、想定通りの文字列が設定されること
    QCOMPARE_EQ(e.message, msg);
}

void TestCommonException::test_clone()
{
    const QString msg = rd.nextQString(length);
    CommonException e(msg);
    // QVERIFYはreturn;する可能性があるため、QScopedPointerに入れる
    const QScopedPointer<QException> copied(e.clone());

    // clone()を呼び出しても元のインスタンスが変わらないこと
    QCOMPARE_EQ(e.message, msg);
    // clone()で返されたインスタンスのメッセージが元のインスタンスと変わらないこと
    QCOMPARE_EQ(static_cast<CommonException *>(copied.get())->message, msg);
    // clone()で返されたインスタンスが元のインスタンスとは異なること
    QCOMPARE_NE(&e, copied.get());
}

void TestCommonException::test_raise()
{
    const QString msg = rd.nextQString(length);
    CommonException src(msg);

    // try-catchのスコープより寿命が長い例外インスタンスを投げる
    try {
        src.raise();
    } catch (CommonException &e) {
        // srcが変更されていないこと
        QCOMPARE_EQ(src.message, msg);

        // eとsrcとの内容が同じであること
        QCOMPARE_EQ(e.message, msg);

        return;
    }

    QFAIL("raise() did not throw CommonException");
}

void TestCommonException::test_QDebugOutputOperator()
{
    CommonException d("Custom message for Debug");
    CommonException i("Custom message for Info");
    CommonException w("Custom message for Waning");

    // NOTE: 想定通りの値が出力できたかはコード上では確認が難しそう
    /* テスト結果に
     * 
     * DEBUG    "Custom message for Debug"
     * INFO     "Custom message for Info"
     * WARN     "Custom message for Waning"
     * 
     * と出力されていれば成功
     */
    qDebug() << d;
    qInfo() << i;
    qWarning() << w;
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestCommonException)

#include "test_common_exception.moc"
