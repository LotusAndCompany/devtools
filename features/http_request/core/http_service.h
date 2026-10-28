#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

struct HttpRequest
{
    QString method;
    QString url;
    QString username;
    QString password;
    QString body;
};

struct HttpResponse
{
    quint64 request_id;
    QByteArray body;
    int status_code;
    QString reason;
    int error_code;
    QString error_message;
    qint64 elapsed_ms;

    bool isSuccess() const;
};

class HttpService : public QObject
{
    Q_OBJECT

public:
    explicit HttpService(QObject *parent = nullptr);
    quint64 send(const HttpRequest &input);
    void cancel(quint64 request_id);
    static QString urlWithParameters(const QString &url,
                                     const QList<QPair<QString, QString>> &parameters);

signals:
    void finished(const HttpResponse &response);

private:
    void handleFinished(QNetworkReply *reply);

    QNetworkAccessManager *manager;
    quint64 next_request_id = 0;
    qint64 request_start_time = 0;
};

Q_DECLARE_METATYPE(HttpResponse)

#endif // HTTP_SERVICE_H
