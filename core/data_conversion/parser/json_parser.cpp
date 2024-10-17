#include "json_parser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

BasicParser::ParseResult JsonParser::tryParse(const QString &src) const
{
    ParseResult result;

    QJsonParseError *jsonError = nullptr;
    QJsonDocument doc = QJsonDocument::fromJson(src.toUtf8(), jsonError);
    if (jsonError == nullptr && !doc.isNull()) {
        if (doc.isObject()) {
            result.type = ParseResult::DataType::MAP;
            result.data = doc.object().toVariantMap();
        } else if (doc.isArray()) {
            result.type = ParseResult::DataType::LIST;
            result.data = doc.array().toVariantList();
        } else {
            // このパターンがあり得るのかは不明。data.typeId()はQMetaType::UnknownTypeになる。
            result.type = ParseResult::DataType::UNKNOWN;
            result.data = QVariant();
        }

        result.success = true;
    } else {
        if (jsonError) {
            result.errors.push_back(jsonError->errorString());
            qInfo() << jsonError->errorString();
        } else {
            result.errors.push_back("Failed to parse JSON (no detail information available)");
            qInfo() << result.errors.back();
        }

        result.type = ParseResult::DataType::UNKNOWN;
        result.data = QVariant();
        result.success = false;
    }

    return result;
}
