#include "json_parser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

JsonParser::ParseResult JsonParser::tryParse(const QString &src) const
{
    ParseResult result;

    QJsonParseError *jsonError = nullptr;
    QJsonDocument const doc = QJsonDocument::fromJson(src.toUtf8(), jsonError);
    if (jsonError == nullptr && !doc.isNull()) {
        if (doc.isObject()) {
            result.data = doc.object().toVariantMap();
        } else if (doc.isArray()) {
            result.data = doc.array().toVariantList();
        } else {
            // このパターンがあり得るのかは不明。data.typeId()はQMetaType::UnknownTypeになる。
            result.data = QVariant();
        }

        result.success = true;
    } else {
        if (jsonError != nullptr) {
            result.errors.push_back(jsonError->errorString());
            qInfo() << jsonError->errorString();
        } else {
            result.errors.push_back("Failed to parse JSON (no detail information available)");
            qInfo() << result.errors.back();
        }

        result.data = QVariant();
        result.success = false;
    }

    return result;
}
