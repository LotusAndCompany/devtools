#include "yaml_emitter.h"

#include "core/exception/invalid_argument_exception.h"

#include <QCoreApplication>

#include <yaml-cpp/yaml.h>

YamlEmitter::EmitResult YamlEmitter::emitQString(const QVariant &data,
                                                 DataConversionInterface::Format style,
                                                 DataConversionInterface::Indentation indentation)
{
    warnings.clear();

    if (data.typeId() == QMetaType::Type::UnknownType) {
        return EmitResult{
            "",
            {},
            QCoreApplication::translate("DataConversion", "Invalid input value"),
        };
    }

    try {
        YAML::Emitter out;
        switch (indentation) {
        case DataConversion::Indentation::MINIFIED:
            out.SetIndent(0);
            break;
        case DataConversionInterface::Indentation::SPACES_2:
            out.SetIndent(2);
            break;
        case DataConversionInterface::Indentation::SPACES_4:
            out.SetIndent(4);
            break;
        default:
            return EmitResult{
                "",
                {},
                QCoreApplication::translate("DataConversion",
                                            "YAML does not support tab indentation"),
            };
        }
        out << variantToYamlNode(data, style, indentation);
        return EmitResult{
            out.c_str() + QString("\n"),
            {},
            "",
        };
    } catch (CommonException &e) {
        qCritical() << e.message;
        return EmitResult{
            "",
            {},
            e.message,
        };
    }
}

YAML::Node YamlEmitter::variantToYamlNode(
    const QVariant &value, DataConversionInterface::Format style,
    DataConversionInterface::Indentation indentation) noexcept(false)
{
    switch (value.typeId()) {
    case QMetaType::Type::QVariantList:
        return variantListToYamlSequence(value.toList(), style, indentation);
    case QMetaType::Type::QVariantMap:
        return variantMapToYamlMap(value.toMap(), style, indentation);
    case QMetaType::Type::UnknownType:
        throw InvalidArgumentException<int>(static_cast<int>(QMetaType::Type::UnknownType),
                                            "value cannot be unkown type");
    default: {
        return YAML::Node(value.toString().toStdString());
    }
    }
}

YAML::Node YamlEmitter::variantListToYamlSequence(const QVariantList &variantList,
                                                  DataConversionInterface::Format style,
                                                  DataConversionInterface::Indentation indentation)
{
    YAML::Node sequence(YAML::NodeType::Sequence);
    switch (style) {
    case DataConversionInterface::Format::YAML_BLOCK:
        sequence.SetStyle(YAML::EmitterStyle::Block);
        break;
    case DataConversionInterface::Format::YAML_FLOW:
        sequence.SetStyle(YAML::EmitterStyle::Flow);
        break;
    default:
        throw InvalidArgumentException<int>(static_cast<int>(style),
                                            "style must be YAML_BLOCK or YAML_FLOW");
    }

    for (const auto &elem : variantList) {
        sequence.push_back(variantToYamlNode(elem, style, indentation));
    }

    return sequence;
}

YAML::Node YamlEmitter::variantMapToYamlMap(const QVariantMap &variantMap,
                                            DataConversionInterface::Format style,
                                            DataConversionInterface::Indentation indentation)
{
    YAML::Node map(YAML::NodeType::Map);
    switch (style) {
    case DataConversionInterface::Format::YAML_BLOCK:
        map.SetStyle(YAML::EmitterStyle::Block);
        break;
    case DataConversionInterface::Format::YAML_FLOW:
        map.SetStyle(YAML::EmitterStyle::Flow);
        break;
    default:
        throw InvalidArgumentException<int>(static_cast<int>(style),
                                            "style must be YAML_BLOCK or YAML_FLOW");
    }

    for (const auto &entry : variantMap.asKeyValueRange()) {
        map[entry.first.toStdString()] = variantToYamlNode(entry.second, style, indentation);
    }

    return map;
}
