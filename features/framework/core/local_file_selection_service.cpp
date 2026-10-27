#include "local_file_selection_service.h"

#include <QFileInfo>

bool LocalFileSelectionService::hasSuffix(const QString &path, const QStringList &suffixes)
{
    return suffixes.contains(QFileInfo(path).suffix(), Qt::CaseInsensitive);
}

QString LocalFileSelectionService::firstExistingLocalFile(const QList<QUrl> &urls,
                                                          const QStringList &suffixes)
{
    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        const QString path = url.toLocalFile();
        const QFileInfo fileInfo(path);
        if (fileInfo.isFile() && hasSuffix(path, suffixes)) {
            return path;
        }
    }
    return {};
}
