#include "tests/test_util.h"

#include <QtTest>

#define _TEST_TomlEmitter
#include "core/data_conversion/emitter/toml_emitter.h"
#undef _TEST_TomlEmitter

#include <QObject>

namespace Test {
class TestTomlEmitter : public QObject
{
    Q_OBJECT

    RandomData rd;
    QTime getRandomQTime();
    QDate getRandomQDate();
    QTimeZone getRandomQTimeZone();

private slots:
    void test_emitQString();
};

QTime TestTomlEmitter::getRandomQTime()
{
    const uint8_t randomHour = rd.nextInt(0, 24), randomMin = rd.nextInt(0, 60),
                  randomSec = rd.nextInt(0, 60);
    const uint16_t randomMsec = rd.nextInt(0, 1000);

    return QTime(randomHour, randomMin, randomSec, randomMsec);
}

QDate TestTomlEmitter::getRandomQDate()
{
    const int16_t randomAD = rd.nextInt(1970, 3000);
    const uint8_t randomMon = rd.nextInt(1, 13), randomDay = rd.nextInt(1, 29);

    return QDate(randomAD, randomMon, randomDay);
}

QTimeZone TestTomlEmitter::getRandomQTimeZone()
{
    const int8_t randomHour = rd.nextInt(-12, 13), randomMin = rd.nextInt(-59, 60);

    return QTimeZone(60 * (60 * randomHour + randomMin));
}

void TestTomlEmitter::test_emitQString()
{
    TomlEmitter emitter;
    TomlEmitter::EmitResult result;

    // 連想配列配列を変換できること
    QVariantMap map = QVariantMap{
        {"t", true},
        {"f", false},
        {"i", rd.nextInt()},
        {"d", 2.25},
        {"s", rd.nextQString(RandomData::numbers + RandomData::lowerAlphabets +
                             RandomData::upperAlphabets)},
        {"a", QVariantList()},
        {"o", QVariantMap()},
    };
    // FIXME: 順番が変わってしまう
    //        出力される順番は謎
    result = emitter.emitQString(map, DataConversion::Indentation::MINIFIED);
    QCOMPARE_EQ(result.text,
                QString("t = true\ns = \"%2\"\na = []\ni = %1\nd = 2.25\nf = false\n\n[o]\n")
                    .arg(map["i"].toInt())
                    .arg(map["s"].toString()));

    map = {
        {"a", 0},
        {"b", QVariantMap{{"c", 1}}},
    };

    // 4スペースで出力されること
    result = emitter.emitQString(map, DataConversion::Indentation::SPACES_4);
    QCOMPARE_EQ(result.text, QString("a = 0\n\n[b]\n    c = 1\n"));

    // 2スペースで出力されること
    result = emitter.emitQString(map, DataConversion::Indentation::SPACES_2);
    QCOMPARE_EQ(result.text, QString("a = 0\n\n[b]\n  c = 1\n"));

    // タブで出力されること
    result = emitter.emitQString(map, DataConversion::Indentation::TABS);
    QCOMPARE_EQ(result.text, QString("a = 0\n\n[b]\n\tc = 1\n"));

    // インデント無しで出力されること
    result = emitter.emitQString(map, DataConversion::Indentation::MINIFIED);
    QCOMPARE_EQ(result.text, QString("a = 0\n\n[b]\nc = 1\n"));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestTomlEmitter)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
// QTEST_GUILESS_MAIN(Test::TestTomlEmitter)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
// QTEST_MAIN(Test::TestTomlEmitter)

#include "test_toml_emitter.moc"
