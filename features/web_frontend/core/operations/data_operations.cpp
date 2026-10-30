#include "data_operations.h"

#include "features/data_conversion/core/data_conversion.h"
#include "features/timestamp_conversion/core/timestamp_conversion.h"
#include "operation_support.h"

#include <QMetaType>
#include <QVariantList>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {

namespace {
constexpr qsizetype MAX_CONVERTED_OUTPUT_BYTES = qsizetype{4} * 1024 * 1024;
} // namespace

FrontendOperationService::Result convertData(const QVariantMap &payload)
{
    QString input;
    if (!readText(payload, QStringLiteral("input"), &input)) {
        return invalidInput(QStringLiteral("Input must be a UTF-8 string no larger than 1 MiB."));
    }

    DataConversion::Format format = DataConversion::Format::JSON;
    const QString formatName = payload.value(QStringLiteral("format")).toString();
    if (formatName == QStringLiteral("yaml-block")) {
        format = DataConversion::Format::YAML_BLOCK;
    } else if (formatName == QStringLiteral("yaml-flow")) {
        format = DataConversion::Format::YAML_FLOW;
    } else if (formatName == QStringLiteral("toml")) {
        format = DataConversion::Format::TOML;
    } else if (formatName != QStringLiteral("json")) {
        return invalidInput(QStringLiteral("Output format must be JSON, YAML or TOML."));
    }

    DataConversion::Indentation indentation = DataConversion::Indentation::SPACES_2;
    const QString indentationName = payload.value(QStringLiteral("indentation")).toString();
    if (indentationName == QStringLiteral("4")) {
        indentation = DataConversion::Indentation::SPACES_4;
    } else if (indentationName == QStringLiteral("tab")) {
        indentation = DataConversion::Indentation::TABS;
    } else if (indentationName == QStringLiteral("minified")) {
        indentation = DataConversion::Indentation::MINIFIED;
    } else if (indentationName != QStringLiteral("2")) {
        return invalidInput(QStringLiteral("Indentation must be 2, 4, tab or minified."));
    }

    DataConversion conversion;
    conversion.setInputText(input);
    if (conversion.messages() == DataConversion::tr("Cannot parse input text")) {
        return invalidInput(conversion.messages());
    }
    conversion.setOutputFormat(format);
    conversion.setIndentation(indentation);
    conversion.updateOutputText();
    if (conversion.messages().contains(QStringLiteral("error"), Qt::CaseInsensitive) &&
        conversion.outputText().isEmpty()) {
        return invalidInput(conversion.messages());
    }
    if (conversion.outputText().toUtf8().size() > MAX_CONVERTED_OUTPUT_BYTES) {
        return {{},
                QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                QStringLiteral("Converted output is larger than 4 MiB."),
                false};
    }
    return {{{QStringLiteral("output"), conversion.outputText()},
             {QStringLiteral("message"), conversion.messages()}},
            {},
            {},
            false};
}

FrontendOperationService::Result convertTimestamp(const QVariantMap &payload)
{
    QString value;
    if (!readText(payload, QStringLiteral("value"), &value)) {
        return invalidInput(QStringLiteral("Timestamp input must be a UTF-8 string."));
    }
    const QString source = payload.value(QStringLiteral("source")).toString();
    QDateTime instant;
    if (source == QStringLiteral("seconds")) {
        instant = TimestampConversion::fromUnixTimestamp(value, TimestampConversion::Unit::Seconds);
    } else if (source == QStringLiteral("milliseconds")) {
        instant =
            TimestampConversion::fromUnixTimestamp(value, TimestampConversion::Unit::Milliseconds);
    } else if (source == QStringLiteral("iso")) {
        instant = TimestampConversion::fromIso8601(value);
    } else {
        return invalidInput(QStringLiteral("Timestamp source is not supported."));
    }
    if (!instant.isValid()) {
        return invalidInput(QStringLiteral("Enter a valid timestamp or ISO-8601 date."));
    }
    return {
        {{QStringLiteral("seconds"),
          TimestampConversion::toUnixTimestamp(instant, TimestampConversion::Unit::Seconds)},
         {QStringLiteral("milliseconds"),
          TimestampConversion::toUnixTimestamp(instant, TimestampConversion::Unit::Milliseconds)},
         {QStringLiteral("utc"), TimestampConversion::toIso8601(instant)},
         {QStringLiteral("local"), TimestampConversion::toIso8601Local(instant)}},
        {},
        {},
        false};
}

} // namespace FrontendOperationHandlers
