#include "http_service.h"

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <exception>

bool HttpResponse::isSuccess() const
{
    return error_code == QNetworkReply::NoError;
}

HttpService::HttpService(QObject *parent)
    : QObject(parent), manager(new QNetworkAccessManager(this))
{
    connect(manager, &QNetworkAccessManager::finished, this, &HttpService::handleFinished);
}

quint64 HttpService::send(const HttpRequest &input)
{
    if (input.method != "GET" && input.method != "POST" && input.method != "PUT" &&
        input.method != "DELETE") {
        return 0;
    }
    request_start_time = QDateTime::currentMSecsSinceEpoch();
    QNetworkRequest request{QUrl(input.url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!input.username.isEmpty() && !input.password.isEmpty()) {
        const QByteArray credentials =
            (input.username + ":" + input.password).toLocal8Bit().toBase64();
        request.setRawHeader("Authorization", "Basic " + credentials);
    }

    QNetworkReply *reply = nullptr;
    if (input.method == "GET") {
        reply = manager->get(request);
    } else if (input.method == "POST") {
        reply = manager->post(request, input.body.toUtf8());
    } else if (input.method == "PUT") {
        reply = manager->put(request, input.body.toUtf8());
    } else {
        reply = manager->deleteResource(request);
    }
    const quint64 request_id = ++next_request_id;
    reply->setProperty("devtoolsRequestId", QVariant::fromValue(request_id));
    return request_id;
}

void HttpService::cancel(quint64 request_id)
{
    for (auto *reply : manager->findChildren<QNetworkReply *>()) {
        if (reply->property("devtoolsRequestId").toULongLong() == request_id) {
            reply->abort();
            return;
        }
    }
}

QString HttpService::urlWithParameters(const QString &url,
                                       const QList<QPair<QString, QString>> &parameters)
{
    QString base_url = url.split('?').at(0);
    QString query;
    for (const auto &parameter : parameters) {
        if (!parameter.first.isEmpty()) {
            if (!query.isEmpty()) {
                query.append('&');
            }
            query.append(QUrl::toPercentEncoding(parameter.first) + '=' +
                         QUrl::toPercentEncoding(parameter.second));
        }
    }
    if (!query.isEmpty()) {
        base_url.append('?').append(query);
    }
    return base_url;
}

void HttpService::handleFinished(QNetworkReply *reply)
{
    if (reply == nullptr) {
        qCritical() << "Received a null reply object.";
        return;
    }
    reply->deleteLater();
    try {
        const qint64 elapsed_ms = QDateTime::currentMSecsSinceEpoch() - request_start_time;
        const HttpResponse response{
            reply->property("devtoolsRequestId").toULongLong(),
            reply->isOpen() ? reply->readAll() : QByteArray{},
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
            reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(),
            static_cast<int>(reply->error()),
            reply->errorString(),
            elapsed_ms,
        };
        emit finished(response);
    } catch (const std::exception &e) {
        qCritical() << "Exception caught during response handling:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception caught during response handling.";
    }
}
