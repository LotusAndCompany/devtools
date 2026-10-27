#ifndef TEMPLATE_REPOSITORY_H
#define TEMPLATE_REPOSITORY_H

#include <QString>

enum class TemplateValidationError { None, EmptyTitle, EmptyContent };

struct TemplateSaveResult
{
    QString filename;
    TemplateValidationError error{TemplateValidationError::None};
    bool written{false};
};

class TemplateRepository
{
public:
    static TemplateValidationError validate(const QString &title, const QString &content);
    static bool hasUnsavedChanges(const QString &filename, const QString &title,
                                  const QString &content,
                                  const QString &directoryPath = QStringLiteral("content"));
    static TemplateSaveResult save(const QString &title, const QString &content,
                                   const QString &currentFilename = QString(),
                                   const QString &directoryPath = QStringLiteral("content"));
    static bool remove(const QString &filename,
                       const QString &directoryPath = QStringLiteral("content"));
};

#endif // TEMPLATE_REPOSITORY_H
