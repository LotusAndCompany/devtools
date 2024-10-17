#include "data_conversion.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <toml.hpp>
#include <yaml-cpp/yaml.h>

#include "core/exception/common_exception.h"
#include "core/exception/invalid_argument_exception.h"
#include "core/exception/invalid_state_exception.h"

DataConversionInterface::DataConversionInterface(QObject *parent)
    : Tool(Tool::ID::DATA_COVERSION, "data-conversion", parent)
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

const QString DataConversion::invalidPointer = "void *p must not be null";

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
        QString content = std::move(stream.readAll());
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

    if (inputText() == "") {
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
    if (inputText() == "") {
        inputFormat = Format::UNKNOWN;
        intermediateData = QVariant();
        outdated = true;
        return;
    }

    ParseResult result;

    // JSONで解析できるか試す
    tryParseJSON(&result);
    if (result.format == Format::JSON) {
        inputFormat = Format::JSON;
        intermediateData = std::move(result.data);
        outdated = true;
        return;
    }

    // YAMLで解析できるか試す
    tryParseYAML(&result);
    if (result.format == Format::YAML_BLOCK || result.format == Format::YAML_FLOW) {
        inputFormat = result.format;
        intermediateData = std::move(result.data);
        outdated = true;
        return;
    }

    // TOMLで解析できるか試す
    tryParseTOML(&result);
    if (result.format == Format::TOML) {
        inputFormat = Format::TOML;
        intermediateData = std::move(result.data);
        outdated = true;
        return;
    }

    // 解析失敗
    inputFormat = Format::ERROR;
    intermediateData = QVariant();
    outdated = true;
}

void DataConversion::validatePointer(void *p)
{
    if (!p)
        throw InvalidArgumentException<nullptr_t>(invalidPointer);
}

void DataConversion::tryParseJSON(ParseResult *result) const
{
    validatePointer(result);

    QJsonParseError *jsonError = nullptr;
    QJsonDocument jsonDoc = std::move(QJsonDocument::fromJson(inputText().toUtf8(), jsonError));
    if (jsonError == nullptr) {
        if (jsonDoc.isArray())
            result->data = std::move(jsonDoc.array().toVariantList());
        else if (jsonDoc.isObject())
            result->data = std::move(jsonDoc.object().toVariantMap());
        else
            result->data = QVariant();

        result->format = Format::JSON;
    } else {
        qInfo() << jsonError->errorString();

        result->data = QVariant();
        result->format = Format::UNKNOWN;
    }
}

void DataConversion::tryParseYAML(ParseResult *result) const
{
    validatePointer(result);

    bool ok = true;
    const std::string stdStr(std::move(inputText().toStdString()));

    try {
        const auto yml = YAML::Load(stdStr);

        if (yml.IsDefined()) {
            yamlNodeToQVariant(yml, &result->data);
            switch (yml.Style()) {
            case YAML::EmitterStyle::Block:
                result->format = Format::YAML_BLOCK;
                break;
            case YAML::EmitterStyle::Flow:
                result->format = Format::YAML_FLOW;
                break;
            default:
                qWarning() << "Unknown yaml style";
                result->format = Format::YAML_BLOCK;
                break;
            }
        } else {
            result->error = "invalid yaml";
            ok = false;
        }
    } catch (YAML::Exception &e) {
        qInfo() << e.what();
        result->error = e.what();
        ok = false;
    } catch (CommonException &e) {
        qInfo() << e.message;
        result->error = e.message;
        ok = false;
    }

    if (!ok) {
        result->data = QVariant();
        result->format = Format::UNKNOWN;
        result->error = "";
        return;
    }
}

bool DataConversion::yamlNodeToQVariant(const YAML::Node &src, QVariant *dst)
{
    validatePointer(dst);

    if (src.IsDefined()) {
        if (src.IsMap()) {
            QVariantMap map;
            yamlMapToQVariantMap(src, &map);
            *dst = std::move(map);
            return true;
        } else if (src.IsSequence()) {
            QVariantList list;
            yamlMapToQVariantList(src, &list);
            *dst = std::move(list);
            return true;
        } else if (src.IsScalar()) {
            QVariant var;
            yamlScalarToQVariant(src, &var);
            *dst = std::move(var);
            return true;
        }
    }

    *dst = QVariant();
    return false;
}

bool DataConversion::yamlScalarToQVariant(const YAML::Node &src, QVariant *dst)
{
    validatePointer(dst);

    if (src.IsDefined()) {
        if (src.IsScalar()) {
            // TODO: データ型を自動で判別する
            *dst = std::move(QString::fromStdString(src.Scalar()));
            return true;
        } else {
            throw InvalidStateException("src must be a scalar node");
        }
    }

    *dst = QVariant();
    return false;
}

bool DataConversion::yamlMapToQVariantMap(const YAML::Node &src, QVariantMap *dst)
{
    validatePointer(dst);

    if (src.IsDefined()) {
        if (src.IsMap()) {
            QVariantMap map;
            bool result = true;
            for (YAML::const_iterator it = src.begin(); it != src.end() && result; it++) {
                const auto &keyNode = it->first;
                // FIXME: エラーメッセージが出せない
                // yamlではkeyは文字列以外も認められるが、今のところは対応しない
                if (keyNode.IsDefined() && keyNode.IsScalar()) {
                    QVariant key, value;
                    result &= yamlScalarToQVariant(keyNode, &key);
                    result &= yamlNodeToQVariant(it->second[keyNode], &value);

                    map[key.toString()] = value;
                } else {
                    // TODO: エラーメッセージを設定する
                    result = false;
                }
            }

            *dst = std::move(map);
            return result;
        } else {
            throw InvalidStateException("src must be a map node");
        }
    }

    *dst = QVariantMap();
    return false;
}

bool DataConversion::yamlMapToQVariantList(const YAML::Node &src, QVariantList *dst)
{
    validatePointer(dst);

    if (src.IsDefined()) {
        if (src.IsSequence()) {
            QVariantList list;
            bool result = true;
            for (YAML::const_iterator it = src.begin(); it != src.end() && result; it++) {
                // FIXME: エラーメッセージが出せない
                if (it->IsDefined()) {
                    QVariant value;
                    result &= yamlNodeToQVariant(*it, &value);

                    list.push_back(value);
                } else {
                    // TODO: エラーメッセージを設定する
                    result = false;
                }
            }

            *dst = std::move(list);
            return result;
        } else {
            throw InvalidStateException("src must be a sequence node");
        }
    }

    *dst = QVariantList();
    return false;
}

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
