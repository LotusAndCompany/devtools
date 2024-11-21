#include "toml_emitter.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QTimeZone>

#include "core/exception/invalid_argument_exception.h"
#include <toml.hpp>

TomlEmitter::EmitResult TomlEmitter::emitQString(const QVariant &data,
                                                 DataConversionInterface::Indentation style)
{
    warnings.clear();

    if (data.typeId() == QMetaType::Type::UnknownType) {
        return EmitResult{
            "",
            {},
            QCoreApplication::instance()->translate("DataConversion", "Invalid input value"),
        };
    }

    try {
        const auto toml = variantToTomlValue("", data, style);
        QString text = QString::fromStdString(toml::format(toml));
        if (text.endsWith("\n\n"))
            text = text.mid(0, text.length() - 1);
        return EmitResult{text, warnings, ""};
    } catch (InvalidArgumentException<int> e) {
        qCritical() << e.message;
        return EmitResult{"", warnings, e.message};
    } catch (std::exception e) {
        if (e.what() == QString("std::exception")
            && data.typeId() == QMetaType::Type::QVariantList) {
            const QString m = QCoreApplication::instance()
                                  ->translate("DataConversion",
                                              "TOML does not support top level array");
            qCritical() << m;
            return EmitResult{"", warnings, m};
        } else {
            qCritical() << e.what();
            return EmitResult{"", warnings, e.what()};
        }
    }
}

namespace _TomlEmitterPrivate {
struct Util
{
    inline static toml::local_time timeToTomlLocalTime(const QTime &time)
    {
        return toml::local_time(time.hour(), time.minute(), time.second(), time.msec());
    }

    inline static toml::local_date dateToTomlLocalDate(const QDate &date)
    {
        return toml::local_date(date.year(),
                                static_cast<toml::month_t>(date.month() - 1),
                                date.day());
    }
};
}; // namespace _TomlEmitterPrivate

using namespace _TomlEmitterPrivate;

TomlEmitter::toml_value_type TomlEmitter::variantToTomlValue(
    const QString &key, const QVariant &value, DataConversionInterface::Indentation style)
{
    switch (value.typeId()) {
    case QMetaType::Type::Nullptr:
        return toml_value_type();
    case QMetaType::Type::Bool:
        return toml_value_type(value.toBool());
    case QMetaType::Type::Short:    // int16_t
    case QMetaType::Type::Int:      // int32_t
    case QMetaType::Type::Long:     // int32_t or int64_t
    case QMetaType::Type::LongLong: // int64_t
        return toml_value_type(value.toLongLong());
    case QMetaType::Type::Float:
    case QMetaType::Type::Double:
        return toml_value_type(value.toDouble());
    case QMetaType::QString:
        return toml_value_type(value.toString().toStdString());
    case QMetaType::Type::QTime:
        return Util::timeToTomlLocalTime(value.toTime());
    case QMetaType::Type::QDate:
        return toml::local_date(Util::dateToTomlLocalDate(value.toDate()));
    case QMetaType::Type::QDateTime: {
        const auto datetime = value.toDateTime();
        const auto date = datetime.date();
        const auto time = datetime.time();
        const auto timeZone = datetime.timeZone();
        if (timeZone == QTimeZone::systemTimeZone()) {
            // システムのタイムゾーンと同じならlocal_datetime扱い
            return toml::local_datetime(Util::dateToTomlLocalDate(date),
                                        Util::timeToTomlLocalTime(time));
        } else {
            // システムのタイムゾーンと違ったらoffset_datetime扱い
            int offsetSeconds = timeZone.offsetFromUtc(datetime);

            return toml::offset_datetime(Util::dateToTomlLocalDate(date),
                                         Util::timeToTomlLocalTime(time),
                                         toml::time_offset(offsetSeconds / 3600,
                                                           (abs(offsetSeconds) % 3600) / 60));
        }
    }
    case QMetaType::Type::QVariantList:
        return listToTomlArray(key, value.toList(), style);
    case QMetaType::Type::QVariantMap:
        return mapToTomlTable(key, value.toMap(), style);
    default:
        throw InvalidArgumentException<int>(static_cast<int>(value.typeId()), "Unsupported type");
    }
}

TomlEmitter::toml_value_type TomlEmitter::listToTomlArray(const QString &key,
                                                          const QVariantList &list,
                                                          DataConversionInterface::Indentation style)
{
    toml_value_type::array_type array;
    for (const auto &elem : list) {
        // null値は無視
        if (!elem.isNull())
            array.push_back(variantToTomlValue(key, elem, style));
        else {
            warnings.push_back(QCoreApplication::instance()
                                   ->translate("DataConversion", "ignore null value in array: %1")
                                   .arg(key));
            qWarning() << warnings.back();
        }
    }

    toml_value_type value = std::move(array);
    if (style == DataConversion::Indentation::MINIFIED) {
        if (value.is_array_of_tables()) {
            value.as_array_fmt().fmt = toml::array_format::array_of_tables;
        } else {
            value.as_array_fmt().fmt = toml::array_format::oneline;
        }
        value.as_array_fmt().indent_type = toml::indent_char::none;
        value.as_array_fmt().body_indent = 0;
        value.as_array_fmt().closing_indent = 0;
    } else {
        if (value.is_array_of_tables())
            value.as_array_fmt().fmt = toml::array_format::array_of_tables;
        else
            value.as_array_fmt().fmt = toml::array_format::multiline;

        if (style == DataConversion::Indentation::TABS) {
            value.as_array_fmt().indent_type = toml::indent_char::tab;
            value.as_array_fmt().body_indent = 1;
            value.as_array_fmt().closing_indent = 0;
        } else {
            value.as_array_fmt().indent_type = toml::indent_char::space;
            value.as_array_fmt().closing_indent = 0;
            switch (style) {
            case DataConversion::Indentation::SPACES_2:
                value.as_array_fmt().body_indent = 2;
                break;
            case DataConversion::Indentation::SPACES_4:
                value.as_array_fmt().body_indent = 4;
                break;
            default:
                // unreachable
                break;
            }
        }
    }

    return value;
}

TomlEmitter::toml_value_type TomlEmitter::mapToTomlTable(const QString &key,
                                                         const QVariantMap &map,
                                                         DataConversionInterface::Indentation style)
{
    toml_value_type::table_type table;
    for (const auto &entry : map.asKeyValueRange()) {
        const auto childKey = key == "" ? entry.first : key + "." + entry.first;
        // null値は無視
        if (!entry.second.isNull())
            table[entry.first.toStdString()] = variantToTomlValue(childKey, entry.second, style);
        else {
            warnings.push_back(QCoreApplication::instance()
                                   ->translate("DataConversion", "ignore null value of key: %1")
                                   .arg(childKey));
            qWarning() << warnings.back();
        }
    }

    toml_value_type value = std::move(table);
    if (style == DataConversion::Indentation::MINIFIED) {
        value.as_table_fmt().indent_type = toml::indent_char::none;
        value.as_table_fmt().body_indent = 0;
        value.as_table_fmt().closing_indent = 0;
        value.as_table_fmt().name_indent = 0;
    } else {
        value.as_table_fmt().fmt = toml::table_format::multiline;
        value.as_table_fmt().closing_indent = 0;
        if (key == "")
            value.as_table_fmt().body_indent = 0;
        else {
            if (style == DataConversion::Indentation::TABS) {
                value.as_table_fmt().indent_type = toml::indent_char::tab;
                value.as_table_fmt().body_indent = 1;
            } else {
                value.as_table_fmt().indent_type = toml::indent_char::space;
                switch (style) {
                case DataConversion::Indentation::SPACES_2:
                    value.as_table_fmt().body_indent = 2;
                    break;
                case DataConversion::Indentation::SPACES_4:
                    value.as_table_fmt().body_indent = 4;
                    break;
                default:
                    // unreachable
                    break;
                }
            }
        }
    }

    return value;
}
