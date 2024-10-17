#include "json_parser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void JsonParser::tryParse(const QString &src, ParseResult *result) const
{
    validatePointer(result);

    QJsonParseError *jsonError = nullptr;
    QJsonDocument doc = QJsonDocument::fromJson(src.toUtf8(), jsonError);
    if (jsonError == nullptr) {
        if (doc.isObject()) {
            result->type = ParseResult::DataType::MAP;
            result->data = doc.object().toVariantMap();
        } else if (doc.isArray()) {
            result->type = ParseResult::DataType::LIST;
            result->data = doc.array().toVariantList();
        } else {
            // このパターンがあり得るのかは不明。data.typeId()はQMetaType::UnknownTypeになる。
            result->type = ParseResult::DataType::VARIANT;
            result->data = QVariant();
        }

        result->success = true;
        result->errors.clear();
    } else {
        qInfo() << jsonError->errorString();

        result->type = ParseResult::DataType::UNKNOWN;
        result->data = QVariant();
        result->success = false;
        result->errors.push_back(jsonError->errorString());
    }
}
