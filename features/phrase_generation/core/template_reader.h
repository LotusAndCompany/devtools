#ifndef TEMPLATE_READER_H
#define TEMPLATE_READER_H

#include <QString>
#include <QStringList>

class TemplateReader
{
public:
    static QStringList filenames(const QString &directoryPath = QStringLiteral("content"));
    static QString load(const QString &filename, QString *title,
                        const QString &directoryPath = QStringLiteral("content"));
};

#endif // TEMPLATE_READER_H
