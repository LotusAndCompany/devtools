#include "features/framework/gui/design_system.h"

#include <QFrame>
#include <QPlainTextEdit>
#include <QtTest>

namespace Test {
class TestDesignSystem : public QObject
{
    Q_OBJECT

private slots:
    void configuresTextControl();
};

void TestDesignSystem::configuresTextControl()
{
    QPlainTextEdit editor;
    DevTools::Ui::configureTextControl(&editor);

    QCOMPARE(editor.frameShape(), QFrame::StyledPanel);
    QCOMPARE(editor.frameShadow(), QFrame::Raised);
    QCOMPARE(DevTools::Ui::previewContentSize(QSize(100, 100)), QSize(80, 80));
    QCOMPARE(DevTools::Ui::previewContentSize(QSize(10, 10)), QSize(1, 1));
}
} // namespace Test

QTEST_MAIN(Test::TestDesignSystem)

#include "test_design_system.moc"
