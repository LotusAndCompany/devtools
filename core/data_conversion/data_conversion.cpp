#include "data_conversion.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

//#include <toml.hpp>
#include <yaml-cpp/yaml.h>

#include "core/data_conversion/parser/yaml_parser.h"
#include "parser/basic_parser.h"
//#include "core/exception/common_exception.h"
#include "core/exception/invalid_argument_exception.h"
//#include "core/exception/invalid_state_exception.h"
#include "parser/json_parser.h"

DataConversionInterface::DataConversionInterface(QObject *parent)
    : Tool(Tool::ID::DATA_CONVERSION, "data-conversion", parent)
{}

void DataConversionInterface::validateFormat(Format format)
{
    const int intValue = static_cast<int>(format);
    const int FORMAT_MIN = static_cast<int>(Format::MIN);
    const int FORMAT_MAX = static_cast<int>(Format::MAX);

    if (intValue <= FORMAT_MIN || FORMAT_MAX <= intValue)
        throw InvalidArgumentException(
            intValue,
            QString("DataConversionInterface::Format must be in range (%1, %2)")
                .arg(FORMAT_MIN)
                .arg(FORMAT_MAX));
}

void DataConversionInterface::validateIndentation(Indentation indentation)
{
    const int intValue = static_cast<int>(indentation);
    const int INDENT_MIN = static_cast<int>(Indentation::MIN);
    const int INDENT_MAX = static_cast<int>(Indentation::MAX);

    if (intValue <= INDENT_MIN || INDENT_MAX <= intValue)
        throw InvalidArgumentException(
            intValue,
            QString("DataConversionInterface::Indentation must be in range (%1, %2)")
                .arg(INDENT_MIN)
                .arg(INDENT_MAX));
}

//const QString DataConversion::invalidPointer = "void *p must not be null";

DataConversion::DataConversion(QObject *parent)
    : DataConversionInterface(parent)
{}

void DataConversion::setInputText(const QString &inputText)
{
    if (_inputText != inputText) {
        _inputText = inputText;
        outdated = true;
    }
}

bool DataConversion::load(const QString &path)
{
    if (!QFileInfo::exists(path))
        return false;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString content = stream.readAll();
        if (inputText() != content) {
            _inputText = std::move(content);
            outdated = true;
        }
        return stream.status() == QTextStream::Status::Ok;
    }

    return false;
}

bool DataConversion::save(const QString &path, bool overwrite) const
{
    if (!overwrite && QFileInfo::exists(path))
        return false;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << outputText();
        return stream.status() == QTextStream::Status::Ok;
    }

    return false;
}

void DataConversion::setOutputFormat(Format format)
{
    validateFormat(format);

    if (outputFormat() != format) {
        _outputFormat = format;
        outdated = true;
    }
}

void DataConversion::setIndentation(Indentation indentation)
{
    validateIndentation(indentation);

    if (this->indentation() != indentation) {
        _indentation = indentation;
        outdated = true;
    }
}

void DataConversion::updateOutputText()
{
    if (!outdated)
        return;

    if (inputText().trimmed() == "") {
        _outputText = "";
    } else {
        switch (outputFormat()) {
        case Format::JSON:
            // TODO
            break;
        case Format::YAML_BLOCK:
        case Format::YAML_FLOW:
            // TODO
            break;
        case Format::TOML:
            // TODO
            break;
        default:
            break;
        }
    }

    outdated = false;
}

void DataConversion::parseInputText()
{
    if (inputText().trimmed() == "") {
        inputFormat = Format::UNKNOWN;
        intermediateData = QVariant();
        outdated = true;
        return;
    }

    BasicParser::ParseResult result;

    // JSONで解析できるか試す
    JsonParser jp;
    result = jp.tryParse(inputText());
    if (result) {
        inputFormat = Format::JSON;
        intermediateData = std::move(result.data);
        outdated = true;
        return;
    }

    // YAMLで解析できるか試す
    YamlParser yp;
    result = yp.tryParse(inputText());
    if (result) {
        if (result.extras[YamlParser::EXTRAS_YAML_STYLE].toInt()
            == static_cast<int>(YAML::EmitterStyle::Flow))
            inputFormat = Format::YAML_FLOW;
        else
            inputFormat = Format::YAML_BLOCK;

        intermediateData = std::move(result.data);
        outdated = true;
        return;
    }

#if 0
    // TOMLで解析できるか試す
    tryParseTOML(&result);
    if (result.format == Format::TOML) {
        inputFormat = Format::TOML;
        intermediateData = std::move(result.data);
        outdated = true;
        return;
    }
#endif

    // 解析失敗
    inputFormat = Format::ERROR;
    intermediateData = QVariant();
    outdated = true;
}

#if 0
void DataConversion::tryParseTOML(ParseResult *result) const
{
    validatePointer(result);

    const std::string stdStr(std::move(inputText().toStdString()));
    try {
        const auto tml = toml::parse_str(stdStr);
        // TODO
    } catch (toml::exception &e) {
        qInfo() << e.what();

        result->data = QVariant();
        result->format = Format::UNKNOWN;
    }
}
#endif
