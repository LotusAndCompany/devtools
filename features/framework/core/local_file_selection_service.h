#ifndef LOCAL_FILE_SELECTION_SERVICE_H
#define LOCAL_FILE_SELECTION_SERVICE_H

#include <QList>
#include <QStringList>
#include <QUrl>

class LocalFileSelectionService
{
public:
    static bool hasSuffix(const QString &path, const QStringList &suffixes);
    static QString firstExistingLocalFile(const QList<QUrl> &urls, const QStringList &suffixes);
};

#endif // LOCAL_FILE_SELECTION_SERVICE_H
