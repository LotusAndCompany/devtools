#include "features/data_conversion/gui/data_conversion_gui.h"

#include <QFile>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QtTest>

class TestDataConversionGui : public QObject
{
    Q_OBJECT

private slots:
    void loadAndSaveThroughService();
};

void TestDataConversionGui::loadAndSaveThroughService()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString inputPath = directory.filePath("input.json");
    const QString input = "{\"answer\":42}";
    QFile inputFile(inputPath);
    QVERIFY(inputFile.open(QIODevice::WriteOnly));
    QCOMPARE(inputFile.write(input.toUtf8()), qint64(input.toUtf8().size()));
    inputFile.close();

    auto *conversion = new DataConversion;
    DataConversionGUI widget(conversion);
    QPlainTextEdit *inputEditor = nullptr;
    QPlainTextEdit *outputEditor = nullptr;
    for (QPlainTextEdit *editor : widget.findChildren<QPlainTextEdit *>()) {
        if (editor->placeholderText() == "Input text") {
            inputEditor = editor;
        } else if (editor->placeholderText() == "Output text") {
            outputEditor = editor;
        }
    }
    QVERIFY(inputEditor != nullptr);
    QVERIFY(outputEditor != nullptr);

    QVERIFY(QMetaObject::invokeMethod(&widget, "loadFileSelected", Qt::DirectConnection,
                                      Q_ARG(QString, inputPath)));
    QCOMPARE(inputEditor->toPlainText(), input);
    QVERIFY(!outputEditor->toPlainText().isEmpty());

    const QString outputPath = directory.filePath("output.json");
    QVERIFY(QMetaObject::invokeMethod(&widget, "saveFileSelected", Qt::DirectConnection,
                                      Q_ARG(QString, outputPath)));
    QFile outputFile(outputPath);
    QVERIFY(outputFile.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(outputFile.readAll()), outputEditor->toPlainText());
}

QTEST_MAIN(TestDataConversionGui)
#include "test_data_conversion_gui.moc"
