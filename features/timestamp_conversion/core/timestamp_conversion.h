#ifndef TIMESTAMP_CONVERSION_H
#define TIMESTAMP_CONVERSION_H

#include <QDateTime>
#include <QString>

#include <cstdint>

/**
 * @brief UNIXタイムスタンプ・日時・ISO8601形式の相互変換ロジック
 * @details いずれの表現もUTCの`QDateTime`を基準時刻として経由して変換する。@n
 *          `fromXxx()`は各表現を基準時刻に変換し、`toXxx()`は基準時刻を各表現に変換する。
 */
class TimestampConversion
{
public:
    /// タイムスタンプの単位
    enum class Unit : uint8_t {
        Seconds,     ///< 秒
        Milliseconds ///< ミリ秒
    };

    /**
     * @brief UNIXタイムスタンプ文字列を基準時刻に変換する
     * @param text タイムスタンプの文字列表現（整数）
     * @param unit タイムスタンプの単位
     * @param[out] ok 変換に成功した場合に`true`が設定される（`nullptr`可）
     * @return 基準時刻（UTC）。失敗した場合は不正な`QDateTime`
     */
    static QDateTime fromUnixTimestamp(const QString &text, Unit unit, bool *ok = nullptr);

    /**
     * @brief ISO8601形式の文字列（例: 2026-09-13T20:32:37.497Z）を基準時刻に変換する
     * @param text ISO8601形式の文字列
     * @param[out] ok 変換に成功した場合に`true`が設定される（`nullptr`可）
     * @return 基準時刻（UTC）。失敗した場合は不正な`QDateTime`
     */
    static QDateTime fromIso8601(const QString &text, bool *ok = nullptr);

    /**
     * @brief ローカルタイムゾーンの日時を基準時刻に変換する
     * @param localDateTime ローカルタイムゾーンの値として解釈する日時
     * @return 基準時刻（UTC）。localDateTimeが不正な場合は不正な`QDateTime`
     */
    static QDateTime fromLocalDateTime(const QDateTime &localDateTime);

    /**
     * @brief UTCの日時を基準時刻に変換する
     * @param utcDateTime UTCの値として解釈する日時
     * @return 基準時刻（UTC）。utcDateTimeが不正な場合は不正な`QDateTime`
     */
    static QDateTime fromUtcDateTime(const QDateTime &utcDateTime);

    /**
     * @brief 基準時刻をUNIXタイムスタンプ文字列に変換する
     * @param utcInstant 基準時刻（UTC）
     * @param unit タイムスタンプの単位
     * @return タイムスタンプの文字列表現。utcInstantが不正な場合は空文字列
     */
    static QString toUnixTimestamp(const QDateTime &utcInstant, Unit unit);

    /**
     * @brief 基準時刻をISO8601形式（UTC, ミリ秒付き）の文字列に変換する
     * @param utcInstant 基準時刻（UTC）
     * @return ISO8601形式の文字列。utcInstantが不正な場合は空文字列
     */
    static QString toIso8601(const QDateTime &utcInstant);

    /**
     * @brief 基準時刻をISO8601形式（ローカルタイムゾーンのUTCオフセット付き、ミリ秒付き）の
     *        文字列に変換する（例: 2026-09-14T05:32:37.497+09:00）
     * @param utcInstant 基準時刻（UTC）
     * @return ISO8601形式の文字列。utcInstantが不正な場合は空文字列
     */
    static QString toIso8601Local(const QDateTime &utcInstant);

    /**
     * @brief 基準時刻をローカルタイムゾーンの日時に変換する
     * @param utcInstant 基準時刻（UTC）
     * @return ローカルタイムゾーンの日時
     */
    static QDateTime toLocalDateTime(const QDateTime &utcInstant);
};

#endif // TIMESTAMP_CONVERSION_H
