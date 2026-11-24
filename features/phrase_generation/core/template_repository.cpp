#include "template_repository.h"

#include "template_reader.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QUuid>

TemplateValidationError TemplateRepository::validate(const QString &title, const QString &content)
{
    if (title.isEmpty()) {
        return TemplateValidationError::EmptyTitle;
    }
    if (content.isEmpty()) {
        return TemplateValidationError::EmptyContent;
    }
    return TemplateValidationError::None;
}

bool TemplateRepository::hasUnsavedChanges(const QString &filename, const QString &title,
                                           const QString &content, const QString &directoryPath)
{
    if (filename.isEmpty()) {
        return !title.isEmpty() || !content.isEmpty();
    }

    QString savedTitle;
    const QString savedContent = TemplateReader::load(filename, &savedTitle, directoryPath);
    return title != savedTitle || content != savedContent;
}

TemplateSaveResult TemplateRepository::save(const QString &title, const QString &content,
                                            const QString &currentFilename,
                                            const QString &directoryPath)
{
    const TemplateValidationError validationError = validate(title, content);
    if (validationError != TemplateValidationError::None) {
        return {{}, validationError, false};
    }

    if (!currentFilename.isEmpty()) {
        QFile oldFile(QDir(directoryPath).filePath(currentFilename));
        if (oldFile.exists()) {
            oldFile.remove();
        }
    }

    QDir().mkpath(directoryPath);
    const QString filename = QUuid::createUuid().toString(QUuid::WithoutBraces) + ".txt";
    QFile file(QDir(directoryPath).filePath(filename));
    bool written = false;
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << title << "\n" << content;
        out.flush();
        written = out.status() == QTextStream::Status::Ok;
        file.close();
    }

    return {filename, TemplateValidationError::None, written};
}

bool TemplateRepository::remove(const QString &filename, const QString &directoryPath)
{
    QFile file(QDir(directoryPath).filePath(filename));
    return file.remove();
}
