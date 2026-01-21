#include "data_conversion.h"

#include "core/exception/invalid_argument_exception.h"
#include "emitter/json_emitter.h"
#include "emitter/toml_emitter.h"
#include "emitter/yaml_emitter.h"
#include "parser/json_parser.h"
#include "parser/toml_parser.h"
#include "parser/yaml_parser.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <toml.hpp>
#include <yaml-cpp/yaml.h>

DataConversionInterface::DataConversionInterface(QObject *parent)
    : Tool(Tool::ID::DATA_CONVERSION, "data-conversion", parent)
{}

QString DataConversionInterface::messages() const
{
    return _messages.join('\n');
}

void DataConversionInterface::validateFormat(Format format)
{
    const int intValue = static_cast<int>(format);
    const int FORMAT_MIN = static_cast<int>(Format::MIN);
    const int FORMAT_MAX = static_cast<int>(Format::MAX);

    if (intValue <= FORMAT_MIN || FORMAT_MAX <= intValue)
        throw InvalidArgumentException(
            intValue, QString("DataConversionInterface::Format must be in range (%1, %2)")
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
            intValue, QString("DataConversionInterface::Indentation must be in range (%1, %2)")
                          .arg(INDENT_MIN)
                          .arg(INDENT_MAX));
}

// const QString DataConversion::invalidPointer = "void *p must not be null";

DataConversion::DataConversion(QObject *parent) : DataConversionInterface(parent) {}

void DataConversion::setInputText(const QString &inputText)
{
    _messages.clear();

    if (_inputText != inputText) {
        _inputText = inputText;
        outdated = true;
    }
    parseInputText();
}

bool DataConversion::load(const QString &path)
{
    if (!QFileInfo::exists(path)) {
        _messages = {tr("Invalid file path")};
        return false;
    }

    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString content = stream.readAll();
        if (inputText() != content) {
            _inputText = std::move(content);
            outdated = true;
        }
        return stream.status() == QTextStream::Status::Ok;
    } else {
        _messages = {tr("Cannot open %1").arg(path)};
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

    _messages.clear();

    if (inputText().trimmed() == "") {
        _outputText = "";
    } else {
        BasicEmitter::EmitResult result;
        switch (outputFormat()) {
        case Format::JSON: {
            JsonEmitter emitter;
            result = emitter.emitQString(intermediateData, indentation());
            _outputText = result.text;
            break;
        }
        case Format::YAML_BLOCK:
        case Format::YAML_FLOW: {
            YamlEmitter emitter;
            result = emitter.emitQString(intermediateData, outputFormat(), indentation());
            _outputText = result.text;
            break;
        }
        case Format::TOML: {
            TomlEmitter emitter;
            result = emitter.emitQString(intermediateData, indentation());
            _outputText = result.text;
            break;
        }
        default:
            break;
        }

        if (result.error != "") {
            _messages = {result.error};
        } else {
            _messages = result.warnings;
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
        _messages = {tr("Parsed as JSON")};
        outdated = true;
        return;
    }

    TomlParser tp;
    // TOMLで解析できるか試す
    result = tp.tryParse(inputText());
    if (result) {
        inputFormat = Format::TOML;
        intermediateData = std::move(result.data);
        _messages = {tr("Parsed as TOML")};
        outdated = true;
        return;
    }

    // YAMLで解析できるか試す
    YamlParser yp;
    result = yp.tryParse(inputText());
    if (result) {
        if (result.extras[YamlParser::EXTRAS_YAML_STYLE].toInt() ==
            static_cast<int>(YAML::EmitterStyle::Flow)) {
            inputFormat = Format::YAML_FLOW;
            _messages = {tr("Parsed as flow style YAML")};
        } else {
            inputFormat = Format::YAML_BLOCK;
            _messages = {tr("Parsed as block style YAML")};
        }

        intermediateData = std::move(result.data);
        outdated = true;
        return;
    }

    // 解析失敗
    inputFormat = Format::ERROR;
    intermediateData = QVariant();
    _messages = {tr("Cannot parse input text")};
    outdated = true;
}
