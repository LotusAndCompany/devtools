#include "features/http_request/core/http_service.h"

#include <QNetworkReply>
#include <QPointer>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>

class TestHttpService : public QObject
{
    Q_OBJECT

private slots:
    void parameters();
    void request_data();
    void request();
    void concurrentAndHttpError();
    void invalidUrl();
    void cancel();
    void destruction();
};

void TestHttpService::parameters()
{
    QCOMPARE(
        HttpService::urlWithParameters(
            "https://example.test/path?old=1",
            {{"name", "日本 語"}, {"", "ignored"}, {"x&y", "="}, {"name", ""}}),
        QString("https://example.test/path?name=%E6%97%A5%E6%9C%AC%20%E8%AA%9E&x%26y=%3D&name="));
    QCOMPARE(HttpService::urlWithParameters("https://example.test/?old=1", {}),
             QString("https://example.test/"));
}

void TestHttpService::request_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<QByteArray>("payload");
    QTest::newRow("get") << "GET" << QByteArray();
    QTest::newRow("post") << "POST" << QByteArray("{\"x\":1}");
    QTest::newRow("put") << "PUT" << QByteArray("{\"x\":1}");
    QTest::newRow("delete") << "DELETE" << QByteArray();
}

void TestHttpService::request()
{
    QFETCH(QString, method);
    QFETCH(QByteArray, payload);
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    QByteArray received;
    connect(&server, &QTcpServer::newConnection, &server, [&] {
        QTcpSocket *socket = server.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, socket, [&, socket] {
            received += socket->readAll();
            const int body_start = received.indexOf("\r\n\r\n");
            if (body_start >= 0 && received.size() >= body_start + 4 + payload.size()) {
                socket->write(
                    "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello");
                socket->disconnectFromHost();
            }
        });
    });
    HttpService service;
    QSignalSpy spy(&service, &HttpService::finished);
    const quint64 id =
        service.send({method, QString("http://127.0.0.1:%1/path").arg(server.serverPort()), "user",
                      "pass", "{\"x\":1}"});
    QVERIFY(id != 0);
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 5000);
    const HttpResponse response = spy.takeFirst().at(0).value<HttpResponse>();
    QCOMPARE(response.request_id, id);
    QCOMPARE(response.status_code, 200);
    QCOMPARE(response.reason, QString("OK"));
    QCOMPARE(response.error_code, int(QNetworkReply::NoError));
    QCOMPARE(response.body, QByteArray("hello"));
    QVERIFY(received.startsWith(method.toLatin1() + " /path HTTP/1.1\r\n"));
    QVERIFY(received.toLower().contains("content-type: application/json\r\n"));
    QVERIFY(received.toLower().contains("authorization: basic dxnlcjpwyxnz\r\n"));
    QCOMPARE(received.mid(received.indexOf("\r\n\r\n") + 4), payload);
}

void TestHttpService::concurrentAndHttpError()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    HttpService service;
    QSignalSpy spy(&service, &HttpService::finished);
    const QString url = QString("http://127.0.0.1:%1/").arg(server.serverPort());
    QCOMPARE(service.send({"PATCH", url, {}, {}, {}}), quint64(0));
    const quint64 first = service.send({"GET", url, {}, {}, {}});
    QTRY_VERIFY_WITH_TIMEOUT(server.hasPendingConnections(), 5000);
    QTcpSocket *first_socket = server.nextPendingConnection();
    const quint64 second = service.send({"GET", url, {}, {}, {}});
    QTRY_VERIFY_WITH_TIMEOUT(server.hasPendingConnections(), 5000);
    QTcpSocket *second_socket = server.nextPendingConnection();
    second_socket->write(
        "HTTP/1.1 404 Not Found\r\nContent-Length: 7\r\nConnection: close\r\n\r\nmissing");
    second_socket->disconnectFromHost();
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 5000);
    const HttpResponse error = spy.at(0).at(0).value<HttpResponse>();
    QCOMPARE(error.request_id, second);
    QCOMPARE(error.status_code, 404);
    QCOMPARE(error.body, QByteArray("missing"));
    QVERIFY(error.error_code != int(QNetworkReply::NoError));
    QVERIFY(!error.error_message.isEmpty());
    first_socket->write("HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nok");
    first_socket->disconnectFromHost();
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 2, 5000);
    const HttpResponse success = spy.at(1).at(0).value<HttpResponse>();
    QCOMPARE(success.request_id, first);
    QCOMPARE(success.body, QByteArray("ok"));
}

void TestHttpService::invalidUrl()
{
    HttpService service;
    QSignalSpy spy(&service, &HttpService::finished);
    const quint64 id = service.send({"GET", "not-a-url", {}, {}, {}});
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 5000);
    const HttpResponse result = spy.takeFirst().at(0).value<HttpResponse>();
    QCOMPARE(result.request_id, id);
    QVERIFY(!result.isSuccess());
    QVERIFY(!result.error_message.isEmpty());
}

void TestHttpService::cancel()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    HttpService service;
    QSignalSpy spy(&service, &HttpService::finished);
    const quint64 id =
        service.send({"GET", QString("http://127.0.0.1:%1/").arg(server.serverPort()), {}, {}, {}});
    QTRY_VERIFY_WITH_TIMEOUT(server.hasPendingConnections(), 5000);
    service.cancel(id);
    QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 5000);
    const HttpResponse response = spy.takeFirst().at(0).value<HttpResponse>();
    QCOMPARE(response.request_id, id);
    QCOMPARE(response.error_code, int(QNetworkReply::OperationCanceledError));
}

void TestHttpService::destruction()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    auto *service = new HttpService;
    const QPointer<HttpService> guard(service);
    service->send({"GET", QString("http://127.0.0.1:%1/").arg(server.serverPort()), {}, {}, {}});
    QTRY_VERIFY_WITH_TIMEOUT(server.hasPendingConnections(), 5000);
    delete service;
    QVERIFY(guard.isNull());
}

QTEST_GUILESS_MAIN(TestHttpService)
#include "test_http_service.moc"
