#include "tool.h"

#include "core/exception/invalid_argument_exception.h"
#include "core/exception/under_development_exception.h"

#include <QEvent>

#include <utility>

const QString Tool::invalidToolIDReason =
    QString("Tool::ID must be in range (%1, %2)").arg(Tool::ID_MIN).arg(Tool::ID_MAX);

Tool::Tool(Tool::ID id, QString stringID, QObject *parent)
    : QObject(parent), id(id), stringID(std::move(stringID)), _translatable(translatable(id))
{
    validateID(id);
}

void Tool::validateID(ID id)
{
    const int intID = static_cast<int>(id);

    if (intID <= ID_MIN || ID_MAX <= intID) {
        throw InvalidArgumentException(intID, invalidToolIDReason);
    }
}

Tool::Translatable Tool::translatable(ID id)
{
    validateID(id);

    // TODO: return Translatable{tr("Tool Name"), tr("Tool description")}
    switch (id) {
    case ID::IMAGE_RESIZE:
        return Translatable{
            tr("Image Resize"),
            tr("Image resizing and scaling"),
        };
    case ID::IMAGE_ROTATION:
        return Translatable{
            tr("Image Rotation"),
            tr("Image rotation and flipping"),
        };
    case ID::IMAGE_DIVISION:
        return Translatable{
            tr("Image Division"),
            tr("Divide image into n x m"),
        };
    case ID::IMAGE_TRANSPARENT:
        return Translatable{
            tr("Image Transparent"),
            tr("Make the selected color transparent"),
        };
    case ID::PHRASE_GENERATION:
        return Translatable{
            tr("Phrase Generation"),
            tr("Generate and manage Phrase"),
        };
    case ID::COMMAND_GENERATION:
        return Translatable{
            tr("Command Generation"),
            tr("Generate command from command list"),
        };
    case ID::DATA_CONVERSION:
        return Translatable{
            tr("Data/Format Conversion"),
            tr("Conversion and formatting JSON/YAML/TOML data"),
        };
    case ID::HTTP_REQUEST:
        return Translatable{tr("HTTP Request"), tr("Send HTTP Request")};
    case ID::QR_CODE_GENERATION:
        return Translatable{tr("QR Code Generation"), tr("Generate QR codes from text or URLs")};
    case ID::DB_TOOL:
        return Translatable{tr("DB Tool"), tr("Provides database-related functionalities")};
    default:
        throw UnderDevelopmentException();
    }
}

bool Tool::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        QObject::event(event);
        _translatable = translatable(id);
        return true;
    default:
        return QObject::event(event);
    }
}
