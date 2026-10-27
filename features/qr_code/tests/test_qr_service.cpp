#include "features/qr_code/core/content_generator.h"
#include "features/qr_code/core/qr_image.h"
#include "features/qr_code/core/qr_validation.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QTemporaryDir>
#include <QTimeZone>
#include <QtTest>

class TestQRService : public QObject
{
    Q_OBJECT

private slots:
    void validation();
    void content();
    void image_data();
    void image();
    void oversized();
};

void TestQRService::validation()
{
    using Code = QRValidationCode;
    QVERIFY(!QRValidation::canGenerate(QRCodeType::Text, {{"text_content", "  "}}));
    QVERIFY(QRValidation::canGenerate(QRCodeType::Url, {{"url_content", "bad"}}));
    QVERIFY(!QRValidation::canGenerate(
        QRCodeType::Wifi, {{"wifi_ssid", "S"}, {"wifi_security", "WPA"}, {"wifi_password", ""}}));
    QVERIFY(QRValidation::canGenerate(QRCodeType::Wifi,
                                      {{"wifi_ssid", "S"}, {"wifi_security", "nopass"}}));
    QVERIFY(!QRValidation::canGenerate(QRCodeType::Calendar, {{"cal_summary", "A"}}));
    auto errors = QRValidation::validate(QRCodeType::Text, {{"text_content", "  "}});
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].code, Code::TextEmpty);
    QCOMPARE(errors[0].field, QString("text_content_error"));
    QVERIFY(QRValidation::validate(QRCodeType::Text, {{"text_content", "日本語"}}).isEmpty());
    errors = QRValidation::validate(QRCodeType::Url, {{"url_content", "HTTP://example.com"}});
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].code, Code::UrlScheme);
    QVERIFY(QRValidation::validate(QRCodeType::Url, {{"url_content", "https://example.com"}})
                .isEmpty());
    errors = QRValidation::validate(QRCodeType::Email, {{"email_address", "invalid"}});
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].code, Code::EmailInvalid);
    QVERIFY(QRValidation::validate(QRCodeType::Email, {{"email_address", "a@b"}}).isEmpty());
    QVERIFY(
        QRValidation::validate(QRCodeType::Phone, {{"phone_number", "090-1234-5678"}}).isEmpty());
    errors = QRValidation::validate(QRCodeType::Phone, {{"phone_number", "abc"}});
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].code, Code::PhoneInvalid);
    errors = QRValidation::validate(QRCodeType::Sms, {});
    QCOMPARE(errors.size(), 2);
    QCOMPARE(errors[0].code, Code::PhoneEmpty);
    QCOMPARE(errors[1].code, Code::MessageEmpty);
    errors =
        QRValidation::validate(QRCodeType::Wifi, {{"wifi_ssid", "SSID"}, {"wifi_security", "WEP"}});
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].code, Code::PasswordEmpty);
    QVERIFY(QRValidation::validate(QRCodeType::Wifi,
                                   {{"wifi_ssid", "SSID"}, {"wifi_security", "nopass"}})
                .isEmpty());
    errors = QRValidation::validate(
        QRCodeType::Contact,
        {{"contact_name", "A"}, {"contact_phone", "bad"}, {"contact_email", "bad"}});
    QCOMPARE(errors.size(), 2);
    QCOMPARE(errors[0].code, Code::PhoneInvalid);
    QCOMPARE(errors[1].code, Code::EmailInvalid);
    errors = QRValidation::validate(QRCodeType::Contact, {{"contact_name", "A"}});
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].code, Code::ContactEmpty);
    const QDateTime time(QDate(2026, 1, 2), QTime(10, 0), QTimeZone::utc());
    errors = QRValidation::validate(QRCodeType::Calendar,
                                    {{"cal_summary", "A"}, {"cal_start", time}, {"cal_end", time}});
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].code, Code::EndBeforeStart);
    QVERIFY(QRValidation::validate(
                QRCodeType::Calendar,
                {{"cal_summary", "A"}, {"cal_start", time}, {"cal_end", time.addSecs(1)}})
                .isEmpty());
    errors = QRValidation::validate(QRCodeType::Calendar, {});
    QCOMPARE(errors.size(), 3);
    QVERIFY(QRValidation::validate(QRCodeType::Geo, {{"geo_lat", "-90"}, {"geo_lng", "180"}})
                .isEmpty());
    errors = QRValidation::validate(QRCodeType::Geo, {{"geo_lat", "90.01"}, {"geo_lng", "abc"}});
    QCOMPARE(errors.size(), 2);
    QCOMPARE(errors[0].code, Code::LatitudeInvalid);
    QCOMPARE(errors[1].code, Code::LongitudeInvalid);
}

void TestQRService::content()
{
    QCOMPARE(ContentGenerator::generate("text", {{"text", "日本語"}}), QString("日本語"));
    QCOMPARE(ContentGenerator::generate("url", {{"url", "https://a"}}), QString("https://a"));
    QCOMPARE(ContentGenerator::generate("email", {{"email", "a@b"}}), QString("mailto:a@b"));
    QCOMPARE(ContentGenerator::generate("phone", {{"number", "123"}}), QString("tel:123"));
    QCOMPARE(ContentGenerator::generate("sms", {{"number", "123"}, {"text", "Hi"}}),
             QString("SMSTO:123:Hi"));
    QCOMPARE(ContentGenerator::generate(
                 "wifi", {{"ssid", "S"}, {"password", "P"}, {"type", "WPA"}, {"hidden", "true"}}),
             QString("WIFI:T:WPA;S:S;P:P;;"));
    QCOMPARE(
        ContentGenerator::generate("contact", {{"name", "A"}, {"phone", "123"}, {"email", "a@b"}}),
        QString("BEGIN:VCARD\nVERSION:3.0\nFN:A\nTEL:123\nEMAIL:a@b\nEND:VCARD"));
    QCOMPARE(
        ContentGenerator::generate("calendar", {{"summary", "A"}, {"start", "1"}, {"end", "2"}}),
        QString("BEGIN:VEVENT\nSUMMARY:A\nDTSTART:1\nDTEND:2\nEND:VEVENT"));
    QCOMPARE(ContentGenerator::generate("geo", {{"lat", "1.5"}, {"lng", "-2"}}),
             QString("geo:1.500000,-2.000000"));
    QVERIFY(ContentGenerator::generate("unknown", {}).isEmpty());
    QVERIFY(ContentGenerator::generate("wifi", {}).isEmpty());
}

void TestQRService::image_data()
{
    QTest::addColumn<QString>("content");
    QTest::addColumn<QByteArray>("expected_hash");
    QTest::newRow("ascii")
        << "hello"
        << QByteArray("260d8ac526494ef509d15206c54fd80b1350657cfa9d7cbe08ac28887e9c77aa");
    QTest::newRow("unicode")
        << QStringLiteral("日本語")
        << QByteArray("a9ac5cabcfa125f4d48701bc50b3ba3a895da50bfb8e4c9126b5ed9ed4486ccc");
}

void TestQRService::image()
{
    QFETCH(QString, content);
    QFETCH(QByteArray, expected_hash);
    const QImage image = QRImage::generate(content);
    QCOMPARE(image.size(), QSize(168, 168));
    QCOMPARE(image.format(), QImage::Format_RGB32);
    const QByteArray bytes(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes());
    QCOMPARE(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex(), expected_hash);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("code.png");
    QVERIFY(QRImage::savePng(image, path));
    QCOMPARE(QImage(path), image);
    QVERIFY(!QRImage::savePng(image, directory.filePath("missing/code.png")));
}

void TestQRService::oversized()
{
    QVERIFY_THROWS_EXCEPTION(std::exception, QRImage::generate(QString(5000, 'x')));
}

QTEST_GUILESS_MAIN(TestQRService)
#include "test_qr_service.moc"
