#include "features/jwt_decoder/core/jwt_decoder.h"

#include <QtTest>

namespace Test {

namespace {
/// alg=HS256 / typ=JWT, sub / name / iat を含む標準的な JWT (jwt.io のサンプル)
const QString VALID_JWT =
    "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
    "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ."
    "SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c";
const QString VALID_SIGNATURE = "SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c";

QString decodeAndGetHeaderJson(JwtDecoder &decoder, const QString &token)
{
    decoder.setInputText(token);
    decoder.decode();

    return decoder.header().formattedJson;
}
} // namespace

class TestJwtDecoder : public QObject
{
    Q_OBJECT

private slots:
    // Test cases:
    static void test_decode_validToken();
    static void test_decode_unpaddedBase64Url();
    static void test_decode_twoSegmentToken();
    static void test_decode_singleSegmentToken();
    static void test_decode_invalidBase64UrlPayload();
    static void test_decode_invalidJsonPayload();
    static void test_decode_emptyInput();
    static void test_decode_resetsPreviousResult();
};

void TestJwtDecoder::test_decode_validToken()
{
    JwtDecoder decoder;
    const QString headerJson = decodeAndGetHeaderJson(decoder, VALID_JWT);

    QVERIFY(decoder.isValidStructure());
    QVERIFY(decoder.header().ok);
    QVERIFY(decoder.payload().ok);
    QVERIFY(decoder.header().error.isEmpty());

    QVERIFY(headerJson.contains("\"alg\""));
    QVERIFY(headerJson.contains("HS256"));
    QVERIFY(decoder.payload().formattedJson.contains("\"sub\""));
    QVERIFY(decoder.payload().formattedJson.contains("John Doe"));

    // 署名はデコードせず Base64URL 文字列のまま保持する
    QCOMPARE(decoder.signature(), VALID_SIGNATURE);
}

void TestJwtDecoder::test_decode_unpaddedBase64Url()
{
    // {"a":1} → "eyJhIjoxfQ" (パディング無し) / {"b":2} → "eyJiIjoyfQ"
    JwtDecoder decoder;
    decoder.setInputText("eyJhIjoxfQ.eyJiIjoyfQ.sig");
    decoder.decode();

    QVERIFY(decoder.header().ok);
    QVERIFY(decoder.payload().ok);
    QVERIFY(decoder.header().formattedJson.contains("\"a\""));
    QVERIFY(decoder.payload().formattedJson.contains("\"b\""));
}

void TestJwtDecoder::test_decode_twoSegmentToken()
{
    JwtDecoder decoder;
    decoder.setInputText("eyJhIjoxfQ.eyJiIjoyfQ");
    decoder.decode();

    QVERIFY(decoder.isValidStructure());
    QVERIFY(decoder.header().ok);
    QVERIFY(decoder.payload().ok);
    QVERIFY(decoder.signature().isEmpty());
}

void TestJwtDecoder::test_decode_singleSegmentToken()
{
    JwtDecoder decoder;
    decoder.setInputText("this-is-not-a-jwt");
    decoder.decode();

    QVERIFY(!decoder.isValidStructure());
    QVERIFY(!decoder.header().ok);
    QVERIFY(!decoder.payload().ok);
    QVERIFY(decoder.signature().isEmpty());
}

void TestJwtDecoder::test_decode_invalidBase64UrlPayload()
{
    // ペイロードだけ Base64URL として不正にする
    JwtDecoder decoder;
    decoder.setInputText("eyJhIjoxfQ.not-valid-base64!!!.sig");
    decoder.decode();

    QVERIFY(decoder.isValidStructure());

    // 失敗したパートだけがエラーになり、他のパートは表示できる
    QVERIFY(!decoder.payload().ok);
    QVERIFY(!decoder.payload().error.isEmpty());
    QVERIFY(decoder.payload().formattedJson.isEmpty());
    QVERIFY(decoder.header().ok);
    QCOMPARE(decoder.signature(), QString("sig"));
}

void TestJwtDecoder::test_decode_invalidJsonPayload()
{
    // "bm90LWpzb24" は Base64URL としては正しいが、中身は "not-json"
    JwtDecoder decoder;
    decoder.setInputText("eyJhIjoxfQ.bm90LWpzb24.sig");
    decoder.decode();

    QVERIFY(decoder.isValidStructure());
    QVERIFY(decoder.header().ok);
    QVERIFY(!decoder.payload().ok);
    QVERIFY(!decoder.payload().error.isEmpty());
}

void TestJwtDecoder::test_decode_emptyInput()
{
    JwtDecoder decoder;

    // 未入力および空白のみの入力でクラッシュしないこと
    decoder.decode();
    QVERIFY(!decoder.isValidStructure());

    decoder.setInputText("   \n  ");
    decoder.decode();

    QVERIFY(!decoder.isValidStructure());
    QVERIFY(!decoder.header().ok);
    QVERIFY(decoder.header().error.isEmpty());
    QVERIFY(decoder.signature().isEmpty());
}

void TestJwtDecoder::test_decode_resetsPreviousResult()
{
    JwtDecoder decoder;
    decoder.setInputText(VALID_JWT);
    decoder.decode();
    QVERIFY(decoder.header().ok);

    // 直前の結果が残らないこと
    decoder.setInputText("broken");
    decoder.decode();

    QVERIFY(!decoder.isValidStructure());
    QVERIFY(!decoder.header().ok);
    QVERIFY(decoder.header().formattedJson.isEmpty());
    QVERIFY(decoder.signature().isEmpty());
}

} // namespace Test

// JwtDecoder は QtCore のみに依存するため QApplication は不要
QTEST_APPLESS_MAIN(Test::TestJwtDecoder)

#include "test_jwt_decoder.moc"
