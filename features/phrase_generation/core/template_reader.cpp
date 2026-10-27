#include "template_reader.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

QStringList TemplateReader::filenames()
{
    QDir const directory("content");
    return directory.entryList(QStringList() << "*.txt", QDir::Files);
}

QString TemplateReader::load(const QString &filename, QString *title)
{
    QFile file("content/" + filename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        if (title != nullptr) {
            *title = in.readLine();
        }
        return in.readAll();
    }
    return "";
}
