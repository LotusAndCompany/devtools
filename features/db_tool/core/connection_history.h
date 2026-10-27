#ifndef CONNECTION_HISTORY_H
#define CONNECTION_HISTORY_H

#include <QJsonObject>
#include <QList>

class QSettings;

class ConnectionHistory
{
public:
    static bool isEmpty(const QSettings &settings);
    static QList<QJsonObject> read(const QSettings &settings);
    static void write(QSettings &settings, const QList<QJsonObject> &history);
    static void add(QSettings &settings, const QJsonObject &connection);
};

#endif // CONNECTION_HISTORY_H
