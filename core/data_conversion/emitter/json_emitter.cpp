#include "json_emitter.h"

#include "core/exception/invalid_argument_exception.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QVariant>

JsonEmitter::EmitResult JsonEmitter::emitQString(const QVariant &data,
                                                 DataConversion::Indentation indentation)
{
    warnings.clear();

    if (data.typeId() == QMetaType::Type::UnknownType) {
        return EmitResult{
            "",
            {},
            QCoreApplication::translate("DataConversion", "Invalid input value"),
        };
    } else if (data.typeId() != QMetaType::Type::QVariantList &&
               data.typeId() != QMetaType::Type::QVariantMap) {
        return EmitResult{
            "",
            {},
            QCoreApplication::translate("DataConversion", "JSON does not support top level value"),
        };
    }

    QJsonDocument const doc = QJsonDocument::fromVariant(data);

    switch (indentation) {
    case DataConversion::Indentation::MINIFIED:
        return EmitResult{doc.toJson(QJsonDocument::JsonFormat::Compact), warnings, ""};
    default:
        const QString json = doc.toJson(QJsonDocument::JsonFormat::Indented);
        switch (indentation) {
        case DataConversion::Indentation::SPACES_4:
            return EmitResult{json, warnings, ""};
        case DataConversion::Indentation::SPACES_2:
            return EmitResult{replace4spaceIndentation(json, "  "), warnings, ""};
        case DataConversion::Indentation::TABS:
            return EmitResult{replace4spaceIndentation(json, "\t"), warnings, ""};
        default:
            // unreachable
            throw InvalidArgumentException<int>(
                "DataConversion::Indentation must be INIFIED, SPACES_4, SPACES_2 or TABS.");
        }
    }
}
