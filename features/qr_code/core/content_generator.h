#ifndef CONTENT_GENERATOR_H
#define CONTENT_GENERATOR_H

#include <QString>
#include <QVariantMap>

class ContentGenerator
{
public:
    static QString generate(const QString &type, const QVariantMap &params);

private:
    static QString generateCalendarEvent(const QVariantMap &params);
    static QString generateContactInfo(const QVariantMap &params);
    static QString generateEmail(const QVariantMap &params);
    static QString generateGeo(const QVariantMap &params);
    static QString generatePhone(const QVariantMap &params);
    static QString generateSms(const QVariantMap &params);
    static QString generateText(const QVariantMap &params);
    static QString generateUrl(const QVariantMap &params);
    static QString generateWifi(const QVariantMap &params);
};

#endif
