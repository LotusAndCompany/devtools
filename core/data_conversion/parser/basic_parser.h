#ifndef BASIC_PARSER_H
#define BASIC_PARSER_H

#include <QVariant>

#ifdef _TEST_BasicParser
namespace Test {
class TestBasicParser;
}
#endif

/**
 * @brief テキストデータを解析するクラスの基底クラス
 */
class BasicParser
{
public:
    /// デストラクタ
    virtual ~BasicParser() = default;

    /// 解析結果を格納する構造体
    struct ParseResult
    {
        /// 成功ならtrue
        bool success = false;
        /// データ
        QVariant data;
        /// 補足情報など
        QVariantMap extras;
        /// エラーメッセージ
        QStringList errors;

        /// 真偽値に変換する
        operator bool() const { return success; }
    };

    /**
     * @brief 入力文字列を解析できるなら解析する
     * @param src 入力文字列
     * @return 結果
     */
    virtual ParseResult tryParse(const QString &src) const = 0;

#ifdef _TEST_BasicParser
    friend class Test::TestBasicParser;
#endif
};

#endif // BASIC_PARSER_H
