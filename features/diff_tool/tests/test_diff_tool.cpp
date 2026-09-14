#include "features/diff_tool/core/diff_tool.h"

#include <QtTest>

namespace Test {
class TestDiffTool : public QObject
{
    Q_OBJECT

private slots:
    static void test_splitLines();
    static void test_diffLines_equal();
    static void test_diffLines_insert();
    static void test_diffLines_delete();
    static void test_diffLines_modify();
    static void test_groupSideBySide();
};

void TestDiffTool::test_splitLines()
{
    QCOMPARE(DiffTool::splitLines(QString()), QStringList());
    QCOMPARE(DiffTool::splitLines(QStringLiteral("a")), QStringList{QStringLiteral("a")});
    QCOMPARE(DiffTool::splitLines(QStringLiteral("a\nb")),
             (QStringList{QStringLiteral("a"), QStringLiteral("b")}));
    QCOMPARE(DiffTool::splitLines(QStringLiteral("a\nb\n")),
             (QStringList{QStringLiteral("a"), QStringLiteral("b")}));
    QCOMPARE(DiffTool::splitLines(QStringLiteral("a\r\nb")),
             (QStringList{QStringLiteral("a"), QStringLiteral("b")}));
    QCOMPARE(DiffTool::splitLines(QStringLiteral("a\rb")),
             (QStringList{QStringLiteral("a"), QStringLiteral("b")}));
    QCOMPARE(DiffTool::splitLines(QStringLiteral("a\n\nb")),
             (QStringList{QStringLiteral("a"), QString(), QStringLiteral("b")}));
}

void TestDiffTool::test_diffLines_equal()
{
    const QStringList left{QStringLiteral("a"), QStringLiteral("b")};
    const QStringList right{QStringLiteral("a"), QStringLiteral("b")};

    const auto lines = DiffTool::diffLines(left, right);
    QCOMPARE(lines.size(), 2);
    QCOMPARE(lines.at(0).operation, DiffOperation::Equal);
    QCOMPARE(lines.at(0).left_line_number, 1);
    QCOMPARE(lines.at(0).right_line_number, 1);
    QCOMPARE(lines.at(1).operation, DiffOperation::Equal);
    QCOMPARE(lines.at(1).left_line_number, 2);
    QCOMPARE(lines.at(1).right_line_number, 2);
}

void TestDiffTool::test_diffLines_insert()
{
    const QStringList left{QStringLiteral("a"), QStringLiteral("b")};
    const QStringList right{QStringLiteral("a"), QStringLiteral("x"), QStringLiteral("b")};

    const auto lines = DiffTool::diffLines(left, right);
    QCOMPARE(lines.size(), 3);
    QCOMPARE(lines.at(0).operation, DiffOperation::Equal);
    QCOMPARE(lines.at(1).operation, DiffOperation::Insert);
    QCOMPARE(lines.at(1).left_line_number, 0);
    QCOMPARE(lines.at(1).right_line_number, 2);
    QCOMPARE(lines.at(1).text, QStringLiteral("x"));
    QCOMPARE(lines.at(2).operation, DiffOperation::Equal);
}

void TestDiffTool::test_diffLines_delete()
{
    const QStringList left{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")};
    const QStringList right{QStringLiteral("a"), QStringLiteral("c")};

    const auto lines = DiffTool::diffLines(left, right);
    QCOMPARE(lines.size(), 3);
    QCOMPARE(lines.at(0).operation, DiffOperation::Equal);
    QCOMPARE(lines.at(1).operation, DiffOperation::Delete);
    QCOMPARE(lines.at(1).left_line_number, 2);
    QCOMPARE(lines.at(1).right_line_number, 0);
    QCOMPARE(lines.at(1).text, QStringLiteral("b"));
    QCOMPARE(lines.at(2).operation, DiffOperation::Equal);
}

void TestDiffTool::test_diffLines_modify()
{
    const QStringList left{QStringLiteral("a"), QStringLiteral("b")};
    const QStringList right{QStringLiteral("a"), QStringLiteral("c")};

    const auto lines = DiffTool::diffLines(left, right);
    QCOMPARE(lines.size(), 3);
    QCOMPARE(lines.at(0).operation, DiffOperation::Equal);
    QCOMPARE(lines.at(1).operation, DiffOperation::Delete);
    QCOMPARE(lines.at(1).text, QStringLiteral("b"));
    QCOMPARE(lines.at(2).operation, DiffOperation::Insert);
    QCOMPARE(lines.at(2).text, QStringLiteral("c"));
}

void TestDiffTool::test_groupSideBySide()
{
    const QStringList left{QStringLiteral("a"), QStringLiteral("b")};
    const QStringList right{QStringLiteral("a"), QStringLiteral("c")};

    const auto rows = DiffTool::groupSideBySide(DiffTool::diffLines(left, right));
    QCOMPARE(rows.size(), 2);
    QCOMPARE(rows.at(0).left_text, QStringLiteral("a"));
    QCOMPARE(rows.at(0).right_text, QStringLiteral("a"));
    QVERIFY(!rows.at(0).left_changed);
    QVERIFY(!rows.at(0).right_changed);
    QCOMPARE(rows.at(1).left_text, QStringLiteral("b"));
    QCOMPARE(rows.at(1).right_text, QStringLiteral("c"));
    QVERIFY(rows.at(1).left_changed);
    QVERIFY(rows.at(1).right_changed);
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestDiffTool)

#include "test_diff_tool.moc"
