#include <QtTest>
#include "core/exception/invalid_argument_exception.h"
#include "tests/test_util.h"

#define _TEST_DataConversion
#include "core/data_conversion/data_conversion.h"
#undef _TEST_DataConversion

#include <QObject>

namespace Test {
class TestDataConversion : public QObject
{
    Q_OBJECT

    RandomData rd;
    const QString testDirName = "test_data_conversion_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";

private slots:
    void initTestCase();    // will be called before the first test function is executed.
    void cleanupTestCase(); // will be called after the last test function was executed.
    void test_constructor();
    void test_validateFormat();
    void test_validateIndentation();
    void test_setInputText();
    void test_load();
    void test_save();
    void test_parseInputText();
};

void TestDataConversion::initTestCase()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);
}

void TestDataConversion::cleanupTestCase()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestDataConversion::test_constructor()
{
    DataConversion dataConversion;

    // 入力/出力文字列の初期値が空であること
    QCOMPARE_EQ(dataConversion._inputText, "");
    QCOMPARE_EQ(dataConversion._outputText, "");

    // 出力形式の初期値がJSONであること
    QCOMPARE_EQ(dataConversion._outputFormat, DataConversion::Format::JSON);
    // インデントの初期値が4スペースであること
    QCOMPARE_EQ(dataConversion._indentation, DataConversion::Indentation::SPACES_4);

    // outdatedの初期値がfalseであること
    QCOMPARE_EQ(dataConversion.outdated, false);

    // inputFormatの初期値がFormat::UNKNOWNであること
    QCOMPARE_EQ(dataConversion.inputFormat, DataConversion::Format::UNKNOWN);

    // 中間データの初期値が空であること
    QVERIFY(dataConversion.intermediateData.isNull());
}

void TestDataConversion::test_validateFormat()
{
    // MIN, MAX以外の値の場合は例外を投げないこと
    QVERIFY_THROWS_NO_EXCEPTION(DataConversion::validateFormat(DataConversion::Format::ERROR));
    QVERIFY_THROWS_NO_EXCEPTION(DataConversion::validateFormat(DataConversion::Format::JSON));
    QVERIFY_THROWS_NO_EXCEPTION(DataConversion::validateFormat(DataConversion::Format::TOML));
    QVERIFY_THROWS_NO_EXCEPTION(DataConversion::validateFormat(DataConversion::Format::UNKNOWN));
    QVERIFY_THROWS_NO_EXCEPTION(DataConversion::validateFormat(DataConversion::Format::YAML_BLOCK));
    QVERIFY_THROWS_NO_EXCEPTION(DataConversion::validateFormat(DataConversion::Format::YAML_FLOW));

    // MIN, MAXの場合は例外を投げること
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<int>,
                             DataConversion::validateFormat(DataConversion::Format::MIN));
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<int>,
                             DataConversion::validateFormat(DataConversion::Format::MAX));
}

void TestDataConversion::test_validateIndentation()
{
    // MIN, MAX以外の値の場合は例外を投げないこと
    QVERIFY_THROWS_NO_EXCEPTION(
        DataConversion::validateIndentation(DataConversion::Indentation::MINIFIED));
    QVERIFY_THROWS_NO_EXCEPTION(
        DataConversion::validateIndentation(DataConversion::Indentation::SPACES_2));
    QVERIFY_THROWS_NO_EXCEPTION(
        DataConversion::validateIndentation(DataConversion::Indentation::SPACES_4));
    QVERIFY_THROWS_NO_EXCEPTION(
        DataConversion::validateIndentation(DataConversion::Indentation::TABS));

    // MIN, MAXの場合は例外を投げること
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<int>,
                             DataConversion::validateIndentation(DataConversion::Indentation::MIN));
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<int>,
                             DataConversion::validateIndentation(DataConversion::Indentation::MAX));
}

void TestDataConversion::test_setInputText()
{
    DataConversion dataConversion;

    QString randomQString = rd.nextQString();

    dataConversion.setInputText(randomQString);
    // inputTextが設定されること
    QCOMPARE_EQ(dataConversion.inputText(), randomQString);
    // outdatedがtrueになること
    QCOMPARE_EQ(dataConversion.outdated, true);

    dataConversion.outdated = false;

    dataConversion.setInputText(randomQString);
    QCOMPARE_EQ(dataConversion.inputText(), randomQString);
    // outdatedが変わらないこと
    QCOMPARE_EQ(dataConversion.outdated, false);

    randomQString = rd.nextQString();
    dataConversion.setInputText(randomQString);
    // inputTextが設定されること
    QCOMPARE_EQ(dataConversion.inputText(), randomQString);
    // outdatedがtrueになること
    QCOMPARE_EQ(dataConversion.outdated, true);
}

void TestDataConversion::test_load()
{
    DataConversion dataConversion;

    // 存在しないファイルを読み込もうとすると失敗すること
    const QString randomFilePath = TEST_SRC_DIR + "/" + rd.nextQString();
    QVERIFY(!dataConversion.load(randomFilePath));
    // inputTextが設定されないこと
    QCOMPARE_EQ(dataConversion.inputText(), "");
    // outdatedが変化しないこと
    QCOMPARE_EQ(dataConversion.outdated, false);

    // ファイルを読み込めること
    QFile file(TEST_SRC_DIR + "/core/data_conversion/test.json");
    QVERIFY(dataConversion.load(file.fileName()));
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        const QString json = stream.readAll();

        // 読み込んだファイルの内容が設定されていること
        QCOMPARE_EQ(dataConversion.inputText(), json);
        // outdatedがtrueになること
        QCOMPARE_EQ(dataConversion.outdated, true);
    } else {
        QFAIL(("failed to load " + file.fileName()).toStdString().c_str());
    }
}

void TestDataConversion::test_save()
{
    DataConversion dataConversion;

    dataConversion._outputText = rd.nextQString();

    const QString savePath = testDirPath + "test_save_0.txt";
    // 保存できること
    QVERIFY(dataConversion.save(savePath));
    QFile file(savePath);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        // 保存した内容がoutputText()と一致すること
        QCOMPARE_EQ(stream.readAll(), dataConversion.outputText());
    } else {
        QFAIL(("failed to load " + file.fileName()).toStdString().c_str());
    }

    dataConversion._outputText = rd.nextQString();
    // overwrite=falseの時は上書きできないこと
    QVERIFY(!dataConversion.save(savePath));

    // overwrite=trueの時は上書きできること
    QVERIFY(dataConversion.save(savePath, true));

    file.close();
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        // 保存した内容がoutputText()と一致すること
        QCOMPARE_EQ(stream.readAll(), dataConversion.outputText());
    } else {
        QFAIL(("failed to load " + file.fileName()).toStdString().c_str());
    }
}

void TestDataConversion::test_parseInputText()
{
    {
        DataConversion dataConversion;

        dataConversion.parseInputText();
        // inputFormatがUNKNOWNになっていること
        QCOMPARE_EQ(dataConversion.inputFormat, DataConversion::Format::UNKNOWN);
        // dataConversion.intermediateDataが空であること
        QCOMPARE_EQ(dataConversion.intermediateData, QVariant());
        // outdatedがtrueになっていること
        QCOMPARE_EQ(dataConversion.outdated, true);
    }

    QFile file(TEST_SRC_DIR + "/core/data_conversion/test.json");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        const QString json = stream.readAll();

        DataConversion dataConversion;
        dataConversion.setInputText(json);

        dataConversion.parseInputText();
        // inputFormatがJSONになっていること
        QCOMPARE_EQ(dataConversion.inputFormat, DataConversion::Format::JSON);
        // dataConversion.intermediateDataが設定されていること
        QVERIFY(dataConversion.intermediateData.isValid());
        // dataConversion.intermediateDataが想定通りであること
        QCOMPARE_EQ(dataConversion.intermediateData.typeId(), QMetaType::Type::QVariantMap);
        // outdatedがtrueになっていること
        QCOMPARE_EQ(dataConversion.outdated, true);
    } else {
        QFAIL(("failed to load " + file.fileName()).toStdString().c_str());
    }

    file.close();
    file.setFileName(TEST_SRC_DIR + "/core/data_conversion/test_block.yaml");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        const QString yaml = stream.readAll();

        DataConversion dataConversion;
        dataConversion.setInputText(yaml);

        dataConversion.parseInputText();
        // inputFormatがYAML_BLOCKになっていること
        QCOMPARE_EQ(dataConversion.inputFormat, DataConversion::Format::YAML_BLOCK);
        // dataConversion.intermediateDataが設定されていること
        QVERIFY(dataConversion.intermediateData.isValid());
        // dataConversion.intermediateDataが想定通りであること
        QCOMPARE_EQ(dataConversion.intermediateData.typeId(), QMetaType::Type::QVariantMap);
        // outdatedがtrueになっていること
        QCOMPARE_EQ(dataConversion.outdated, true);
    } else {
        QFAIL(("failed to load " + file.fileName()).toStdString().c_str());
    }

    file.close();
    file.setFileName(TEST_SRC_DIR + "/core/data_conversion/test_flow.yaml");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        const QString yaml = stream.readAll();

        DataConversion dataConversion;
        dataConversion.setInputText(yaml);

        dataConversion.parseInputText();
        // inputFormatがYAML_FLOWになっていること
        QCOMPARE_EQ(dataConversion.inputFormat, DataConversion::Format::YAML_FLOW);
        // dataConversion.intermediateDataが設定されていること
        QVERIFY(dataConversion.intermediateData.isValid());
        // dataConversion.intermediateDataが想定通りであること
        QCOMPARE_EQ(dataConversion.intermediateData.typeId(), QMetaType::Type::QVariantMap);
        // outdatedがtrueになっていること
        QCOMPARE_EQ(dataConversion.outdated, true);
    } else {
        QFAIL(("failed to load " + file.fileName()).toStdString().c_str());
    }

    file.close();
    file.setFileName(TEST_SRC_DIR + "/core/data_conversion/test.toml");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        const QString toml = stream.readAll();

        DataConversion dataConversion;
        dataConversion.setInputText(toml);

        dataConversion.parseInputText();
        // inputFormatがJSONになっていること
        QCOMPARE_EQ(dataConversion.inputFormat, DataConversion::Format::TOML);
        // dataConversion.intermediateDataが設定されていること
        QVERIFY(dataConversion.intermediateData.isValid());
        // dataConversion.intermediateDataが想定通りであること
        QCOMPARE_EQ(dataConversion.intermediateData.typeId(), QMetaType::Type::QVariantMap);
        // outdatedがtrueになっていること
        QCOMPARE_EQ(dataConversion.outdated, true);
    } else {
        QFAIL(("failed to load " + file.fileName()).toStdString().c_str());
    }
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestDataConversion)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestDataConversion)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_MAIN(Test::TestDataConversion)

#include "test_data_conversion.moc"
