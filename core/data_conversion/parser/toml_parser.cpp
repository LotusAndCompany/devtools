#include "toml_parser.h"

#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QTimeZone>

#include <toml.hpp>

TomlParser::ParseResult TomlParser::tryParse(const QString &src) const
{
    ParseResult result;

    try {
        const auto tml = toml::parse_str(src.toStdString());
        result = tomlValueToQvariant(tml);
    } catch (toml::exception &e) {
        result.success = false;
        result.data = QVariant();
        result.errors.push_back(e.what());
        qInfo() << "toml::exception:" << e.what();
    }

    return result;
}

namespace TomlParserPrivate {
/// TomlParserの内部用ユーティリティ
struct Util
{
    /**
     * @brief tomlの時間型を `QTime` に変換する
     * @param time tomlの時間型の値
     * @return `QTime` 型の値
     */
    static QTime tomlTimeToQTime(const TomlParser::toml_value_type::local_time_type &time)
    {
        if (0 < time.microsecond || 0 < time.nanosecond) {
            qInfo() << "microsecond and nanosecond are ignored";
        }
        return {time.hour, time.minute, time.second, time.millisecond};
    }

    /**
     * @brief tomlの日付型を `QDate` に変換する
     * @param date tomlの日付型の値
     * @return `QDate` 型の値
     */
    static QDate tomlDateToQDate(const TomlParser::toml_value_type::local_date_type &date)
    {
        return {date.year, date.month + 1, date.day};
    }

    /**
     * @brief tomlのタイムゾーンを `QTimeZone` に変換する
     * @param offset tomlのタイムゾーン
     * @return `QTimeZone` 型の値
     */
    static QTimeZone tomlTimeOffsetToQTimeZone(const toml::time_offset &offset)
    {
        // 秒を渡す
        return QTimeZone(60 * (60 * offset.hour + offset.minute));
    }

    /**
     * @brief value の場所を文字列にする
     * @param value tomlの値
     * @return 文字列
     */
    static QString valueLocation(const TomlParser::toml_value_type &value)
    {
        const auto l = value.location();
        return QString(" at line: %1, column: %2")
            .arg(l.first_line_number())
            .arg(l.first_column_number());
    }
};
} // namespace TomlParserPrivate

using namespace TomlParserPrivate;

// NOLINTNEXTLINE(readability-function-size)
TomlParser::ParseResult TomlParser::tomlValueToQvariant(const toml_value_type &value)
{
    if (value.is_array()) {
        return tomlArrayToQvariantList(value.as_array());
    } else if (value.is_table()) {
        return tomlTableToQvariantMap(value.as_table());
    }

    ParseResult result;
    if (value.is_boolean()) {
        result.data = value.as_boolean();
        result.success = true;
    } else if (value.is_floating()) {
        result.data = value.as_floating();
        result.success = true;
    } else if (value.is_integer()) {
        result.data = value.as_integer();
        result.success = true;
    } else if (value.is_string()) {
        result.data = QString::fromStdString(value.as_string());
        result.success = true;
    } else if (value.is_local_date()) {
        const auto localDate = value.as_local_date();
        result.data = Util::tomlDateToQDate(localDate);
        result.success = result.data.toDate().isValid();
    } else if (value.is_local_datetime()) {
        const auto localDateTime = value.as_local_datetime();
        const QDate date = Util::tomlDateToQDate(localDateTime.date);
        const QTime time = Util::tomlTimeToQTime(localDateTime.time);
        // システムのタイムゾーンを設定する
        result.data = QDateTime(date, time, QTimeZone::systemTimeZone());
        result.success = result.data.toDateTime().isValid();
    } else if (value.is_local_time()) {
        const auto localTime = value.as_local_time();
        result.data = Util::tomlTimeToQTime(localTime);
        result.success = result.data.toTime().isValid();
    } else if (value.is_offset_datetime()) {
        const auto offsetDateTime = value.as_offset_datetime();
        const QDate date = Util::tomlDateToQDate(offsetDateTime.date);
        const QTime time = Util::tomlTimeToQTime(offsetDateTime.time);
        const QTimeZone timeZone = Util::tomlTimeOffsetToQTimeZone(offsetDateTime.offset);
        result.data = QDateTime(date, time, timeZone);
        result.success = result.data.toDateTime().isValid();
    } else {
        // value.is_empty()?
        result.data = QVariant();
        result.success = false;
        result.errors.push_back("unknown data" + Util::valueLocation(value));
    }

    return result;
}

TomlParser::ParseResult TomlParser::tomlTableToQvariantMap(const toml_value_type &value)
{
    ParseResult result;

    if (value.is_table()) {
        QVariantMap map;
        const auto &table = value.as_table();
        for (const auto &entry : table) {
            const QString key = QString::fromStdString(entry.first);
            const auto valueParseResult = tomlValueToQvariant(entry.second);
            if (valueParseResult) {
                map[key] = valueParseResult.data;
            } else {
                result.success = false;
                result.data = map;
                const QString msg = "invalid value" + Util::valueLocation(value);
                qInfo() << msg;
                result.errors.push_back(msg);
                return result;
            }
        }

        result.success = true;
        result.data = map;
    } else {
        result.success = false;
        result.data = QVariant();
        const QString msg = "value is expected to be table type" + Util::valueLocation(value);
        qInfo() << msg;
        result.errors.push_back(msg);
    }

    return result;
}

TomlParser::ParseResult TomlParser::tomlArrayToQvariantList(const toml_value_type &value)
{
    ParseResult result;

    if (value.is_array()) {
        QVariantList list;
        const auto &array = value.as_array();
        for (const auto &item : array) {
            const auto valueParseResult = tomlValueToQvariant(item);
            if (valueParseResult) {
                list.append(valueParseResult.data);
            } else {
                result.success = false;
                result.data = list;
                const QString msg = "invalid value" + Util::valueLocation(value);
                qInfo() << msg;
                result.errors.push_back(msg);
                return result;
            }
        }

        result.success = true;
        result.data = list;
    } else {
        result.success = false;
        result.data = QVariant();
        const QString msg = "value is expected to be array type" + Util::valueLocation(value);
        qInfo() << msg;
        result.errors.push_back(msg);
    }

    return result;
}
