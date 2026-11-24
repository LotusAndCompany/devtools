#include "features/phrase_generation/core/template_reader.h"
#include "features/phrase_generation/gui/phrase_generation.h"

#include <QDir>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QtTest>

namespace {
class CurrentPathRestore
{
public:
    CurrentPathRestore() : path(QDir::currentPath()) {}
    ~CurrentPathRestore() { QDir::setCurrent(path); }

private:
    const QString path;
};
} // namespace

class TestPhraseGenerationGui : public QObject
{
    Q_OBJECT

private slots:
    void savesUpdatesAndReloadsTemplate();
};

void TestPhraseGenerationGui::savesUpdatesAndReloadsTemplate()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    CurrentPathRestore restoreCurrentPath;
    QVERIFY(QDir::setCurrent(directory.path()));

    phraseGeneration widget;
    auto *title = widget.findChild<QLineEdit *>("templateTitle");
    auto *content = widget.findChild<QPlainTextEdit *>("templateText");
    auto *templates = widget.findChild<QListWidget *>("titleListWidget");
    QVERIFY(title != nullptr);
    QVERIFY(content != nullptr);
    QVERIFY(templates != nullptr);

    title->setText("First title");
    content->setPlainText("First content");
    QVERIFY(QMetaObject::invokeMethod(&widget, "handleSaveButtonClick", Qt::DirectConnection));
    QCOMPARE(templates->count(), 1);
    const QString firstFilename = templates->item(0)->data(Qt::UserRole).toString();
    QVERIFY(QFileInfo::exists(QDir("content").filePath(firstFilename)));

    title->setText("Updated title");
    content->setPlainText("Updated content");
    QVERIFY(QMetaObject::invokeMethod(&widget, "handleSaveButtonClick", Qt::DirectConnection));
    QCOMPARE(templates->count(), 1);
    const QString updatedFilename = templates->item(0)->data(Qt::UserRole).toString();
    QVERIFY(updatedFilename != firstFilename);
    QVERIFY(!QFileInfo::exists(QDir("content").filePath(firstFilename)));

    QVERIFY(QMetaObject::invokeMethod(&widget, "handleTitleListWidgetItemClick",
                                      Qt::DirectConnection,
                                      Q_ARG(QListWidgetItem *, templates->item(0))));
    QCOMPARE(title->text(), QString("Updated title"));
    QCOMPARE(content->toPlainText(), QString("Updated content"));
    QCOMPARE(TemplateReader::filenames(), QStringList({updatedFilename}));
}

QTEST_MAIN(TestPhraseGenerationGui)
#include "test_phrase_generation_gui.moc"
