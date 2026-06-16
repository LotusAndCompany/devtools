#include "tests/test_util.h"

#include <QWidget>
#include <QtTest>

#define _TEST_SectionFrame
#include "gui/components/section_frame.h"
#undef _TEST_SectionFrame

namespace Test {
class TestSectionFrame : public QObject
{
    Q_OBJECT

private slots:
    static void test_constructorDefaults();
    static void test_setExpanded();
    static void test_setBody();
    static void test_setBodyNullptr();
    static void test_setBodySamePointer();
    static void test_expandedChangedSignal();
};

void TestSectionFrame::test_constructorDefaults()
{
    SectionFrame frame(QStringLiteral("Test Section"));
    QCOMPARE(frame.isExpanded(), true);
}

void TestSectionFrame::test_setExpanded()
{
    SectionFrame frame(QStringLiteral("Test"));
    frame.setExpanded(false);
    QCOMPARE(frame.isExpanded(), false);

    frame.setExpanded(true);
    QCOMPARE(frame.isExpanded(), true);
}

void TestSectionFrame::test_setBody()
{
    SectionFrame frame(QStringLiteral("Test"));
    auto *body = new QWidget();
    frame.setBody(body);
    QCOMPARE(frame.body(), body);
}

void TestSectionFrame::test_setBodyNullptr()
{
    SectionFrame frame(QStringLiteral("Test"));
    frame.setBody(nullptr);
    QCOMPARE(frame.body(), nullptr);
}

void TestSectionFrame::test_setBodySamePointer()
{
    SectionFrame frame(QStringLiteral("Test"));
    auto *body = new QWidget();
    frame.setBody(body);
    QCOMPARE(frame.body(), body);
    frame.setBody(body);
    QCOMPARE(frame.body(), body);
}

void TestSectionFrame::test_expandedChangedSignal()
{
    SectionFrame frame(QStringLiteral("Test"));
    bool received = false;
    bool receivedValue = false;

    connect(&frame, &SectionFrame::expandedChanged, [&](bool expanded) {
        received = true;
        receivedValue = expanded;
    });

    frame.setExpanded(false);
    QCOMPARE(received, true);
    QCOMPARE(receivedValue, false);
}
} // namespace Test

QTEST_MAIN(Test::TestSectionFrame)

#include "test_section_frame.moc"
