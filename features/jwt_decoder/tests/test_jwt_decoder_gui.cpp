#include "features/jwt_decoder/core/jwt_decoder.h"
#include "features/jwt_decoder/gui/jwt_decoder_gui.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QScrollBar>
#include <QString>
#include <QTextBrowser>
#include <QTextEdit>
#include <QtTest>

namespace Test {

namespace {
/// デバウンス (180ms) より十分長く待つ
constexpr int DECODE_WAIT_MS = 400;
/// スクロール位置が戻されないことを確認する待ち時間
constexpr int IDLE_WAIT_MS = 1000;

QString encodeSegment(const QJsonObject &object)
{
    return QString::fromLatin1(
        QJsonDocument(object)
            .toJson(QJsonDocument::Compact)
            .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

/// ペイロードがペインに収まらない長さのトークンを作る
QString longToken()
{
    QJsonObject payload;
    for (int i = 0; i < 60; ++i) {
        payload.insert(QString("claim_%1").arg(i, 2, 10, QChar('0')), QString("value-%1").arg(i));
    }

    return encodeSegment({{"alg", "HS256"}, {"typ", "JWT"}}) + "." + encodeSegment(payload) +
           ".dummy-signature";
}

/// いずれかのペインが指定のテキストを含むか
bool anyViewContains(const QList<QTextBrowser *> &views, const QString &text)
{
    return std::any_of(views.begin(), views.end(), [&text](const QTextBrowser *view) {
        return view->toPlainText().contains(text);
    });
}
} // namespace

class TestJwtDecoderGUI : public QObject
{
    Q_OBJECT

private slots:
    static void test_panes_updateOnInput();
    void test_scrollPosition_isKeptWhileIdle();
};

void TestJwtDecoderGUI::test_panes_updateOnInput()
{
    JwtDecoderGUI gui(new JwtDecoder());
    auto *input = gui.findChild<QTextEdit *>();
    QVERIFY(input != nullptr);

    const QList<QTextBrowser *> views = gui.findChildren<QTextBrowser *>();
    QCOMPARE(views.size(), 3);

    input->setPlainText(
        "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
        "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ."
        "SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c");
    QTest::qWait(DECODE_WAIT_MS);

    QVERIFY(anyViewContains(views, "HS256"));
    QVERIFY(anyViewContains(views, "John Doe"));
    QVERIFY(anyViewContains(views, "SflKxwRJ"));

    // 形式が不正になったら 3 ペインとも空表示に戻る
    input->setPlainText("this-is-not-a-jwt");
    QTest::qWait(DECODE_WAIT_MS);

    for (const QTextBrowser *view : views) {
        QVERIFY(view->toPlainText().contains(QString::fromUtf8("—")));
    }

    // 失敗したパートだけがエラーになり、他のパートは表示される
    input->setPlainText("eyJhIjoxfQ.not-valid-base64!!!.sig");
    QTest::qWait(DECODE_WAIT_MS);

    QVERIFY(anyViewContains(views, "Failed to Base64URL-decode"));
    QVERIFY(anyViewContains(views, "\"a\""));
}

void TestJwtDecoderGUI::test_scrollPosition_isKeptWhileIdle()
{
    JwtDecoderGUI gui(new JwtDecoder());
    gui.resize(1000, 700);
    gui.show();
    QVERIFY(QTest::qWaitForWindowExposed(&gui));

    auto *input = gui.findChild<QTextEdit *>();
    input->setPlainText(longToken());
    QTest::qWait(DECODE_WAIT_MS);

    // スクロールバーが出るのは行数の多いペイロードペインだけ
    QTextBrowser *payloadView = nullptr;
    for (QTextBrowser *view : gui.findChildren<QTextBrowser *>()) {
        if (view->verticalScrollBar()->maximum() > 0) {
            payloadView = view;
        }
    }
    QVERIFY(payloadView != nullptr);

    // NOTE: シンタックスハイライトの書式変更でも textChanged が飛ぶため、以前は
    //       デコード → 再ハイライト → デコードのループでビューが作り直され、
    //       ユーザーのスクロール位置が先頭に戻されていた。
    int textChangedCount = 0;
    connect(input, &QTextEdit::textChanged, [&textChangedCount]() { ++textChangedCount; });

    QScrollBar *scrollBar = payloadView->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    const int scrolledTo = scrollBar->value();
    QVERIFY(scrolledTo > 0);

    QTest::qWait(IDLE_WAIT_MS);

    QCOMPARE(scrollBar->value(), scrolledTo);
    QCOMPARE(textChangedCount, 0);
}

} // namespace Test

QTEST_MAIN(Test::TestJwtDecoderGUI)

#include "test_jwt_decoder_gui.moc"
