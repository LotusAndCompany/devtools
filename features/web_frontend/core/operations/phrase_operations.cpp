#include "phrase_operations.h"

#include "features/phrase_generation/core/template_reader.h"
#include "features/phrase_generation/core/template_repository.h"
#include "operation_support.h"

#include <QMetaType>
#include <QRegularExpression>
#include <QVariantList>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {

bool validTemplateFilename(const QString &filename)
{
    static const QRegularExpression pattern(QStringLiteral(
        "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\.txt$"));
    return pattern.match(filename).hasMatch();
}

FrontendOperationService::Result phraseData(const QVariantMap &payload)
{
    const QString action = payload.value(QStringLiteral("action")).toString();
    if (action == QStringLiteral("list")) {
        QVariantList templates;
        const QStringList filenames = TemplateReader::filenames();
        templates.reserve(filenames.size());
        for (const QString &filename : filenames) {
            QString title;
            const QString content = TemplateReader::load(filename, &title);
            templates.append(QVariantMap{{QStringLiteral("filename"), filename},
                                         {QStringLiteral("title"), title},
                                         {QStringLiteral("content"), content}});
        }
        return {{{QStringLiteral("templates"), templates}}, {}, {}, false};
    }
    if (action == QStringLiteral("save")) {
        QString title;
        QString content;
        if (!readText(payload, QStringLiteral("title"), &title) ||
            !readText(payload, QStringLiteral("content"), &content)) {
            return invalidInput(
                QStringLiteral("Template title and content must be strings no larger than 1 MiB."));
        }
        const QString filename = payload.value(QStringLiteral("filename")).toString();
        if (!filename.isEmpty() && !validTemplateFilename(filename)) {
            return invalidInput(QStringLiteral("Template identifier is invalid."));
        }
        const TemplateSaveResult saved = TemplateRepository::save(title, content, filename);
        if (saved.error != TemplateValidationError::None || !saved.written) {
            return invalidInput(saved.error == TemplateValidationError::EmptyTitle
                                    ? QStringLiteral("Template title is required.")
                                : saved.error == TemplateValidationError::EmptyContent
                                    ? QStringLiteral("Template content is required.")
                                    : QStringLiteral("Could not save the template."));
        }
        return {{{QStringLiteral("filename"), saved.filename}}, {}, {}, false};
    }
    if (action == QStringLiteral("delete")) {
        const QString filename = payload.value(QStringLiteral("filename")).toString();
        if (!validTemplateFilename(filename) || !TemplateRepository::remove(filename)) {
            return invalidInput(QStringLiteral("Could not delete the selected template."));
        }
        return {{{QStringLiteral("deleted"), true}}, {}, {}, false};
    }
    return invalidInput(QStringLiteral("Template action is not supported."));
}

} // namespace FrontendOperationHandlers
