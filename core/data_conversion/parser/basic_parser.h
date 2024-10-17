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
        /// 格納されているデータ型を表す列挙体
        enum class DataType {
            MIN,     ///< 最小値
            UNKNOWN, ///< 未設定
            VARIANT, ///< QVariant
            MAP,     ///< QVariantMap
            LIST,    ///< QVariantList
            MAX,     ///< 最大値
        } type
            = DataType::UNKNOWN; ///< 格納されているデータ型
        /// エラーメッセージ
        QStringList errors;
    };

    /**
     * @brief 入力文字列を解析できるなら解析する
     * @param src 入力文字列
     * @param result 結果
     */
    virtual void tryParse(const QString &src, ParseResult *result) const = 0;

protected:
    /**
     * @brief pがnullptrなら例外を投げる
     * @param p チェック対象
     */
    static void validatePointer(void *p) noexcept(false);

#ifdef _TEST_BasicParser
    friend class Test::TestBasicParser;
#endif
};

#endif // BASIC_PARSER_H
