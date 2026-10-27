#include "features/image/gui/basic/image_tools_unified_gui.h"

#include <QLabel>
#include <QMetaObject>
#include <QTemporaryDir>
#include <QtTest>

class TestImageProcessingGui : public QObject
{
    Q_OBJECT

private slots:
    void loadRotateAndSave();
};

void TestImageProcessingGui::loadRotateAndSave()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    QImage source(4, 2, QImage::Format_RGBA8888);
    source.fill(Qt::red);
    const QString sourcePath = directory.filePath("source.png");
    QVERIFY(source.save(sourcePath));

    ImageToolsUnifiedGUI widget;
    QVERIFY(QMetaObject::invokeMethod(&widget, "onLoadImageSelected", Qt::DirectConnection,
                                      Q_ARG(QString, sourcePath)));
    const auto hasLabelText = [&widget](const QString &text) {
        for (const QLabel *label : widget.findChildren<QLabel *>()) {
            if (label->text() == text) {
                return true;
            }
        }
        return false;
    };
    QVERIFY(hasLabelText("4 x 2"));

    QVERIFY(QMetaObject::invokeMethod(&widget, "onRotateRightButtonClicked", Qt::DirectConnection));
    QVERIFY(hasLabelText("2 x 4"));

    const QString savedPath = directory.filePath("rotated.png");
    QVERIFY(QMetaObject::invokeMethod(&widget, "onSaveImageSelected", Qt::DirectConnection,
                                      Q_ARG(QString, savedPath)));
    QCOMPARE(QImage(savedPath).size(), QSize(2, 4));
}

QTEST_MAIN(TestImageProcessingGui)
#include "test_image_processing_gui.moc"
