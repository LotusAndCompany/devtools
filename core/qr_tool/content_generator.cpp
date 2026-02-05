#include "content_generator.h"

#include <QDateTime>

QString ContentGenerator::generate(const QString &type, const QVariantMap &params)
{
    if (type == "calendar") {
        return generateCalendarEvent(params);
    }
    if (type == "contact") {
        return generateContactInfo(params);
    }
    if (type == "email") {
        return generateEmail(params);
    }
    if (type == "geo") {
        return generateGeo(params);
    }
    if (type == "phone") {
        return generatePhone(params);
    }
    if (type == "sms") {
        return generateSms(params);
    }
    if (type == "text") {
        return generateText(params);
    }
    if (type == "url") {
        return generateUrl(params);
    }
    if (type == "wifi") {
        return generateWifi(params);
    }

    return {};
}

QString ContentGenerator::generateCalendarEvent(const QVariantMap &params)
{
    // 必須: summary, start, end
    if (!params.contains("summary") || !params.contains("start") || !params.contains("end")) {
        return {};
    }

    QString const summary = params["summary"].toString();
    QString const start = params["start"].toString();
    QString const end = params["end"].toString();

    return QString("BEGIN:VEVENT\nSUMMARY:%1\nDTSTART:%2\nDTEND:%3\nEND:VEVENT")
        .arg(summary, start, end);
}

QString ContentGenerator::generateContactInfo(const QVariantMap &params)
{
    // 必須: name
    if (!params.contains("name")) {
        return {};
    }

    QString const name = params["name"].toString();
    QString const phone = params.value("phone").toString();
    QString const email = params.value("email").toString();

    QString vcard = QString("BEGIN:VCARD\nVERSION:3.0\nFN:%1").arg(name);
    if (!phone.isEmpty()) {
        vcard += QString("\nTEL:%1").arg(phone);
    }
    if (!email.isEmpty()) {
        vcard += QString("\nEMAIL:%1").arg(email);
    }
    vcard += "\nEND:VCARD";

    return vcard;
}

QString ContentGenerator::generateEmail(const QVariantMap &params)
{
    if (!params.contains("email")) {
        return {};
    }
    QString const email = params["email"].toString();
    return QString("mailto:%1").arg(email);
}

QString ContentGenerator::generateGeo(const QVariantMap &params)
{
    if (!params.contains("lat") || !params.contains("lng")) {
        return {};
    }
    double const lat = params["lat"].toDouble();
    double const lng = params["lng"].toDouble();
    return QString("geo:%1,%2").arg(lat, 0, 'f', 6).arg(lng, 0, 'f', 6);
}

QString ContentGenerator::generatePhone(const QVariantMap &params)
{
    if (!params.contains("number")) {
        return {};
    }
    return QString("tel:%1").arg(params["number"].toString());
}

QString ContentGenerator::generateSms(const QVariantMap &params)
{
    if (!params.contains("number")) {
        return {};
    }
    QString const text = params.value("text").toString();
    return QString("SMSTO:%1:%2").arg(params["number"].toString(), text);
}

QString ContentGenerator::generateText(const QVariantMap &params)
{
    return params.value("text").toString();
}

QString ContentGenerator::generateUrl(const QVariantMap &params)
{
    if (!params.contains("url")) {
        return {};
    }
    return params["url"].toString();
}

QString ContentGenerator::generateWifi(const QVariantMap &params)
{
    // 必須: ssid
    if (!params.contains("ssid")) {
        return {};
    }

    QString const ssid = params["ssid"].toString();
    QString const password = params.value("password").toString();
    QString const type = params.value("type", "WPA").toString(); // WPA/WEP/nopass

    return QString("WIFI:T:%1;S:%2;P:%3;;").arg(type, ssid, password);
}
