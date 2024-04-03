#include <QTest>

#include "core/enum_cast.h"

namespace Test {
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

// NOTE: これはコメントアウト解除してコンパイルエラーになれば想定通り
/*
class TestStaticEnumCast
{
    void cast_lt_Enum_MIN()
    {
        // Enum::MIN未満の数値のキャストが失敗すること
        static_enum_cast<Enum, -1>::result;
    }

    void cast_Enum_MIN()
    {
        // Enum::MINと同じ数値のキャストが失敗すること
        static_enum_cast<Enum, 0>::result;
    }

    void cast_Enum_MAX()
    {
        // Enum::MAXと同じ数値のキャストが失敗すること
        static_enum_cast<Enum, 2>::result;
    }

    void cast_gt_Enum_MAX()
    {
        // Enum::MAX超の数値のキャストが失敗すること
        static_enum_cast<Enum, 3>::result;
    }

    // Tests for EnumClass
    void cast_lt_EnumClass_MIN()
    {
        // EnumClass::MIN未満の数値のキャストが失敗すること
        static_enum_cast<EnumClass, 15>::result;
    }

    void cast_EnumClass_MIN()
    {
        // EnumClass::MINと同じ数値のキャストが失敗すること
        static_enum_cast<EnumClass, 16>::result;
    }

    void cast_EnumClass_MAX()
    {
        // EnumClass::MAXと同じ数値のキャストが失敗すること
        static_enum_cast<EnumClass, 18>::result;
    }

    void cast_gt_EnumClass_MAX()
    {
        // EnumClass::MAX超の数値のキャストが失敗すること
        static_enum_cast<EnumClass, 19>::result;
    }
};
*/

class TestEnumCast : public QObject
{
    Q_OBJECT
private:
    using exception_type = InvalidArgumentException<int>;
private slots:
    // Tests for Enum
    void cast_lt_Enum_MIN()
    {
        // Enum::MIN未満の数値のキャストが失敗すること
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<Enum>(-1));
    }

    void cast_Enum_MIN()
    {
        // Enum::MINと同じ数値のキャストが失敗すること
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<Enum>(0));
    }

    void cast_Enum_VALUE()
    {
        // Enum::VALUEと同じ数値のキャストが成功すること
        const Enum result = static_enum_cast<Enum, 1>::result;
        QCOMPARE_EQ(result, Enum::VALUE);

        QCOMPARE_EQ(enum_cast<Enum>(1), Enum::VALUE);
    }

    void cast_Enum_MAX()
    {
        // Enum::MAXと同じ数値のキャストが失敗すること
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<Enum>(2));
    }

    void cast_gt_Enum_MAX()
    {
        // Enum::MAX超の数値のキャストが失敗すること
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<Enum>(3));
    }

    // Tests for EnumClass
    void cast_lt_EnumClass_MIN()
    {
        // EnumClass::MIN未満の数値のキャストが失敗すること
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<EnumClass>(15));
    }

    void cast_EnumClass_MIN()
    {
        // EnumClass::MINと同じ数値のキャストが失敗すること
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<EnumClass>(16));
    }

    void cast_EnumClass_VALUE()
    {
        // EnumClass::VALUEと同じ数値のキャストが成功すること
        const EnumClass result = static_enum_cast<EnumClass, 17>::result;
        QCOMPARE_EQ(result, EnumClass::VALUE);

        QCOMPARE_EQ(enum_cast<EnumClass>(17), EnumClass::VALUE);
    }

    void cast_EnumClass_MAX()
    {
        // EnumClass::MAXと同じ数値のキャストが失敗すること
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<EnumClass>(18));
    }

    void cast_gt_EnumClass_MAX()
    {
        // EnumClass::MAX超の数値のキャストが失敗すること
        QVERIFY_THROWS_EXCEPTION(exception_type, enum_cast<EnumClass>(19));
    }
};
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestEnumCast)

#include "test_enum_cast.moc"
