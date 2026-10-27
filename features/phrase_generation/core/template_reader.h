#ifndef TEMPLATE_READER_H
#define TEMPLATE_READER_H

#include <QString>
#include <QStringList>

class TemplateReader
{
public:
    static QStringList filenames();
    static QString load(const QString &filename, QString *title);
};

#endif // TEMPLATE_READER_H
