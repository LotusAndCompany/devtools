#include "json_emitter.h"

#include <QJsonDocument>
#include <QVariant>

#include "core/exception/invalid_argument_exception.h"

QString JsonEmitter::emitQString(const QVariant &data, DataConversion::Indentation style) const
{
    if (data.typeId() == QMetaType::Type::UnknownType) {
        return "";
    }

    QJsonDocument doc = QJsonDocument::fromVariant(data);

    switch (style) {
    case DataConversion::Indentation::MINIFIED:
        return doc.toJson(QJsonDocument::JsonFormat::Compact);
    default:
        QString json = doc.toJson(QJsonDocument::JsonFormat::Indented);
        switch (style) {
        case DataConversion::Indentation::SPACES_4:
            return json;
        case DataConversion::Indentation::SPACES_2:
            return replace4spaceIndentation(json, "  ");
        case DataConversion::Indentation::TABS:
            return replace4spaceIndentation(json, "\t");
        default:
            // unreachable
            throw InvalidArgumentException<int>(
                "DataConversion::Indentation must be INIFIED, SPACES_4, SPACES_2 or TABS.");
        }
    }
}
