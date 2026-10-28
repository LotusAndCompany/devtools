#include "template_reader.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

QStringList TemplateReader::filenames(const QString &directoryPath)
{
    QDir const directory(directoryPath);
    return directory.entryList(QStringList() << "*.txt", QDir::Files);
}

QString TemplateReader::load(const QString &filename, QString *title, const QString &directoryPath)
{
    QFile file(QDir(directoryPath).filePath(filename));
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        if (title != nullptr) {
            *title = in.readLine();
        }
        return in.readAll();
    }
    return "";
}
