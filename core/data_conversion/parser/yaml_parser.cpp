#include "yaml_parser.h"

#include "core/exception/invalid_argument_exception.h"

#include <yaml-cpp/yaml.h>

const QString YamlParser::EXTRAS_YAML_STYLE = "yaml_style";

YamlParser::ParseResult YamlParser::tryParse(const QString &src) const
{
    ParseResult result;

    try {
        const auto yml = YAML::Load(src.toStdString());
        result = yamlNodeToQVariant(yml);
    } catch (YAML::Exception &e) {
        result.success = false;
        result.data = QVariant();
        result.errors.push_back(e.what());
        qInfo() << "YAML::Exception:" << e.what();
    } catch (CommonException &e) {
        result.success = false;
        result.data = QVariant();
        result.errors.push_back(e.message);
        qInfo() << "CommonException:" << e.message;
    }

    return result;
}

/**
 * @brief `node` の場所を文字列として返す
 * @param node 対象のノード
 * @return `node` の場所(" at position: %1, line: %2, column: %3" の形)
 */
QString _nodePosition(const YAML::Node &node)
{
    const auto &mark = node.Mark();
    return QString(" at position: %1, line: %2, column: %3")
        .arg(mark.pos)
        .arg(mark.line)
        .arg(mark.column);
}

bool YamlParser::validateYamlNode(const YAML::Node &node, ParseResult *result)
{
    if (!node.IsDefined()) {
        if (result) {
            result->data = QVariant();
            result->success = false;
            result->errors.push_back("invalid node" + _nodePosition(node));
        }
        return false;
    }
    return true;
}

YamlParser::ParseResult YamlParser::yamlNodeToQVariant(const YAML::Node &node)
{
    ParseResult result;

    if (validateYamlNode(node, &result)) {
        if (node.IsMap())
            return yamlMapToQVariantMap(node);
        else if (node.IsSequence())
            return yamlSequenceToQVariantList(node);
        else if (node.IsScalar())
            return yamlScalarToQVariant(node);
        else if (node.IsNull()) {
            // NOTE: このパターンがあり得るのか分からない
            result.success = true;
            result.data = QVariant::fromValue(nullptr);
        }
    }

    return result;
}

YamlParser::ParseResult YamlParser::yamlScalarToQVariant(const YAML::Node &node)
{
    ParseResult result;

    if (validateYamlNode(node, &result)) {
        if (node.IsScalar()) {
            // TODO: データ型を自動で判別する
            result.success = true;
            result.data = QString::fromStdString(node.Scalar());
            return result;
        } else {
            // 論理エラー; 到達不能の想定
            throw InvalidArgumentException<bool>(
                node.IsScalar(), "node is expected to be scalar type" + _nodePosition(node));
        }
    }

    return result;
}

YamlParser::ParseResult YamlParser::yamlMapToQVariantMap(const YAML::Node &node)
{
    ParseResult result;

    if (validateYamlNode(node, &result)) {
        if (node.IsMap()) {
            QVariantMap map;
            for (YAML::const_iterator it = node.begin(); it != node.end(); it++) {
                const auto &keyNode = it->first;
                if (keyNode.IsScalar()) {
                    const auto &valueNode = it->second;
                    const auto keyParseResult = yamlScalarToQVariant(keyNode);
                    const auto valueParseResult = yamlNodeToQVariant(valueNode);

                    if (keyParseResult && valueParseResult) {
                        map[keyParseResult.data.toString()] = std::move(valueParseResult.data);
                    } else {
                        // keyもしくはvalueが不正
                        result.success = false;
                        result.data = std::move(map);
                        result.extras[EXTRAS_YAML_STYLE] = static_cast<int>(node.Style());
                        result.errors.append(std::move(keyParseResult.errors));
                        result.errors.append(std::move(valueParseResult.errors));
                        return result;
                    }
                } else {
                    // NOTE: yamlでは文字列以外のkeyも認められるが、今のところはサポートしない
                    result.success = false;
                    result.data = std::move(map);
                    result.extras[EXTRAS_YAML_STYLE] = static_cast<int>(node.Style());
                    result.errors.push_back("no-scaler type key is not supported currently" +
                                            _nodePosition(node));
                    return result;
                }
            }

            result.success = true;
            result.data = std::move(map);
            result.extras[EXTRAS_YAML_STYLE] = static_cast<int>(node.Style());
            return result;
        } else {
            // 論理エラー; 到達不能の想定
            throw InvalidArgumentException<bool>(
                node.IsMap(), "node is expected to be map type" + _nodePosition(node));
        }
    }

    return result;
}

YamlParser::ParseResult YamlParser::yamlSequenceToQVariantList(const YAML::Node &node)
{
    ParseResult result;

    if (validateYamlNode(node, &result)) {
        if (node.IsSequence()) {
            QVariantList list;
            for (YAML::const_iterator it = node.begin(); it != node.end(); it++) {
                const auto valueParseResult = yamlNodeToQVariant(*it);

                if (valueParseResult) {
                    list.push_back(std::move(valueParseResult.data));
                } else {
                    // valueが不正
                    result.success = false;
                    result.data = std::move(list);
                    result.extras[EXTRAS_YAML_STYLE] = static_cast<int>(node.Style());
                    result.errors.append(std::move(valueParseResult.errors));
                    return result;
                }
            }

            result.success = true;
            result.data = std::move(list);
            result.extras[EXTRAS_YAML_STYLE] = static_cast<int>(node.Style());
            return result;
        } else {
            // 論理エラー; 到達不能の想定
            throw InvalidArgumentException(
                node.IsSequence(), "node is expected to be sequence type" + _nodePosition(node));
        }
    }

    return result;
}
