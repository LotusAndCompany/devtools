#ifndef CONTENT_GENERATOR_H
#define CONTENT_GENERATOR_H

#include <QString>
#include <QVariantMap>

class ContentGenerator
{
public:
    QString generate(const QString &type, const QVariantMap &params);

private:
    QString generateCalendarEvent(const QVariantMap &params);
    QString generateContactInfo(const QVariantMap &params);
    QString generateEmail(const QVariantMap &params);
    QString generateGeo(const QVariantMap &params);
    QString generatePhone(const QVariantMap &params);
    QString generateSms(const QVariantMap &params);
    QString generateText(const QVariantMap &params);
    QString generateUrl(const QVariantMap &params);
    QString generateWifi(const QVariantMap &params);
};

#endif
