#include "connection_history.h"

#include <QJsonDocument>
#include <QSettings>
#include <QStringList>

namespace {
constexpr int MAX_HISTORY_ENTRIES = 10;
constexpr auto HISTORY_KEY = "db_tool/connectionHistory";
} // namespace

bool ConnectionHistory::isEmpty(const QSettings &settings)
{
    return settings.value(HISTORY_KEY).toStringList().isEmpty();
}

QList<QJsonObject> ConnectionHistory::read(const QSettings &settings)
{
    QList<QJsonObject> result;
    const QStringList history = settings.value(HISTORY_KEY).toStringList();
    for (const QString &entry : history) {
        const QJsonDocument document = QJsonDocument::fromJson(entry.toUtf8());
        if (!document.isNull() && document.isObject()) {
            result.append(document.object());
        }
    }
    return result;
}

void ConnectionHistory::write(QSettings &settings, const QList<QJsonObject> &history)
{
    QStringList entries;
    for (const QJsonObject &connection : history) {
        entries.append(QString::fromUtf8(QJsonDocument(connection).toJson(QJsonDocument::Compact)));
    }
    settings.setValue(HISTORY_KEY, entries);
}

void ConnectionHistory::add(QSettings &settings, const QJsonObject &connection)
{
    QStringList history = settings.value(HISTORY_KEY).toStringList();
    const QString database = connection["database"].toString();
    const QString type = connection["type"].toString();
    const QString host = connection["host"].toString();
    for (int i = 0; i < history.size(); ++i) {
        const QJsonDocument existing = QJsonDocument::fromJson(history[i].toUtf8());
        if (!existing.isNull() && existing.isObject()) {
            const QJsonObject object = existing.object();
            if (object["database"].toString() == database && object["type"].toString() == type &&
                object["host"].toString() == host) {
                history.removeAt(i);
                break;
            }
        }
    }
    history.prepend(QString::fromUtf8(QJsonDocument(connection).toJson(QJsonDocument::Compact)));
    while (history.size() > MAX_HISTORY_ENTRIES) {
        history.removeLast();
    }
    settings.setValue(HISTORY_KEY, history);
}
