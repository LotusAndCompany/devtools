#include <QTest>

#include "core/enum_cast.h"

namespace Test {
class TestEnumCast : public QObject
{
    Q_OBJECT
public:
private:
    enum Enum {
        MIN,   // 0
        VALUE, // 1
        MAX,   // 2
    };

    enum class EnumClass {
        MIN = 16, // 16
        VALUE,    // 17
        MAX,      // 18
    };

    using exception_type = InvalidArgumentException<int>;
private slots:
    // Tests for Enum
    void cast_lt_Enum_MIN() { QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<Enum>(-1)); }

    void cast_Enum_MIN() { QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<Enum>(0)); }

    void cast_Enum_VALUE() { QCOMPARE_EQ(enum_cast<Enum>(1), Enum::VALUE); }

    void cast_Enum_MAX() { QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<Enum>(2)); }

    void cast_gt_Enum_MAX() { QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<Enum>(3)); }

    // Tests for EnumClass
    void cast_lt_EnumClass_MIN()
    {
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<EnumClass>(15));
    }

    void cast_EnumClass_MIN()
    {
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<EnumClass>(16));
    }

    void cast_EnumClass_VALUE() { QCOMPARE_EQ(enum_cast<EnumClass>(17), EnumClass::VALUE); }

    void cast_EnumClass_MAX()
    {
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<EnumClass>(18));
    }

    void cast_gt_EnumClass_MAX()
    {
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<EnumClass>(19));
    }
};
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestEnumCast)

#include "test_enum_cast.moc"
