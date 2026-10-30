#include "http_operations.h"

#include "features/http_request/core/http_service.h"
#include "operation_support.h"

#include <QElapsedTimer>
#include <QEventLoop>
#include <QMetaType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>
#include <QUrl>
#include <QVariantList>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {

FrontendOperationService::Result sendHttpRequest(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress)
{
    QString urlText;
    QString method;
    QString body;
    if (!readText(payload, QStringLiteral("url"), &urlText) ||
        !readText(payload, QStringLiteral("method"), &method) ||
        !readText(payload, QStringLiteral("body"), &body)) {
        return invalidInput(
            QStringLiteral("URL, method and body must be strings no larger than 1 MiB."));
    }
    static const QStringList methods = {QStringLiteral("GET"), QStringLiteral("POST"),
                                        QStringLiteral("PUT"), QStringLiteral("DELETE")};
    const QVariantList parameterValues = payload.value(QStringLiteral("parameters")).toList();
    if (parameterValues.size() > 10) {
        return invalidInput(QStringLiteral("At most 10 query parameters are supported."));
    }
    QList<QPair<QString, QString>> parameters;
    for (const QVariant &value : parameterValues) {
        const QVariantMap parameter = value.toMap();
        if (parameter.value(QStringLiteral("key")).metaType().id() != QMetaType::QString ||
            parameter.value(QStringLiteral("value")).metaType().id() != QMetaType::QString ||
            parameter.value(QStringLiteral("description")).metaType().id() != QMetaType::QString) {
            return invalidInput(QStringLiteral("Query parameters must contain string fields."));
        }
        parameters.append({parameter.value(QStringLiteral("key")).toString(),
                           parameter.value(QStringLiteral("value")).toString()});
    }
    if (!parameters.isEmpty()) {
        urlText = HttpService::urlWithParameters(urlText, parameters);
    }
    const QUrl url(urlText);
    if (!methods.contains(method) || !url.isValid() ||
        (url.scheme() != QStringLiteral("http") && url.scheme() != QStringLiteral("https")) ||
        url.host().isEmpty()) {
        return invalidInput(QStringLiteral("Enter a valid HTTP or HTTPS URL and method."));
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setTransferTimeout(30'000);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    const QVariantMap headers = payload.value(QStringLiteral("headers")).toMap();
    for (auto it = headers.cbegin(); it != headers.cend(); ++it) {
        const QString key = it.key().trimmed();
        const QString value = it.value().toString();
        static const QRegularExpression headerName(QStringLiteral("^[!#$%&'*+.^_`|~0-9A-Za-z-]+$"));
        if (!headerName.match(key).hasMatch() || value.contains(QLatin1Char('\r')) ||
            value.contains(QLatin1Char('\n')) ||
            key.compare(QStringLiteral("Host"), Qt::CaseInsensitive) == 0 ||
            key.compare(QStringLiteral("Content-Length"), Qt::CaseInsensitive) == 0) {
            return invalidInput(QStringLiteral("A request header is invalid or restricted."));
        }
        request.setRawHeader(key.toUtf8(), value.toUtf8());
    }
    const QString username = payload.value(QStringLiteral("username")).toString();
    const QString password = payload.value(QStringLiteral("password")).toString();
    if (!username.isEmpty() && !password.isEmpty()) {
        request.setRawHeader(QByteArrayLiteral("Authorization"),
                             QByteArrayLiteral("Basic ") +
                                 (username + QLatin1Char(':') + password).toUtf8().toBase64());
    }
    QElapsedTimer elapsed;
    elapsed.start();
    progress(5, QStringLiteral("Sending HTTP request"));
    QNetworkReply *reply = nullptr;
    if (method == QStringLiteral("GET")) {
        reply = manager.get(request);
    } else if (method == QStringLiteral("POST")) {
        reply = manager.post(request, body.toUtf8());
    } else if (method == QStringLiteral("PUT")) {
        reply = manager.put(request, body.toUtf8());
    } else {
        reply = manager.deleteResource(request);
    }
    QByteArray responseBody;
    bool tooLarge = false;
    QObject::connect(reply, &QIODevice::readyRead, reply, [&]() {
        responseBody.append(reply->readAll());
        if (responseBody.size() > 4 * 1024 * 1024) {
            tooLarge = true;
            reply->abort();
        }
    });
    QEventLoop loop;
    QTimer cancellationTimer;
    cancellationTimer.setInterval(50);
    QObject::connect(&cancellationTimer, &QTimer::timeout, &loop, [&]() {
        if (cancellation->load(std::memory_order_relaxed)) {
            reply->abort();
        }
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    cancellationTimer.start();
    loop.exec();
    responseBody.append(reply->readAll());
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    const QString error = reply->errorString();
    const int errorCode = static_cast<int>(reply->error());
    const qint64 elapsedMs = elapsed.elapsed();
    reply->deleteLater();
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }
    if (tooLarge || responseBody.size() > 4 * 1024 * 1024) {
        return {{},
                QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                QStringLiteral("HTTP response body is limited to 4 MiB."),
                false};
    }
    progress(95, QStringLiteral("Preparing HTTP response"));
    return {{{QStringLiteral("status"), status},
             {QStringLiteral("reason"), reason},
             {QStringLiteral("elapsedMs"), elapsedMs},
             {QStringLiteral("errorCode"), errorCode},
             {QStringLiteral("error"), error},
             {QStringLiteral("body"), QString::fromUtf8(responseBody)},
             {QStringLiteral("bytes"), responseBody.size()}},
            {},
            {},
            false};
}

} // namespace FrontendOperationHandlers
