#include "features/http_request/gui/api_tool.h"

#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableView>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>

class TestApiTool : public QObject
{
    Q_OBJECT

private slots:
    void sendsAndDisplaysResponse();
    void closeDuringRequest();
};

void TestApiTool::sendsAndDisplaysResponse()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    QByteArray received;
    connect(&server, &QTcpServer::newConnection, &server, [&] {
        QTcpSocket *socket = server.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, socket, [&, socket] {
            received += socket->readAll();
            if (received.contains("\r\n\r\n")) {
                socket->write(
                    "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello");
                socket->disconnectFromHost();
            }
        });
    });
    api_tool widget;
    QLineEdit *url = widget.findChild<QLineEdit *>();
    QTableView *table = widget.findChild<QTableView *>();
    QPlainTextEdit *response = widget.findChild<QPlainTextEdit *>("responseTextEdit");
    QVERIFY(url != nullptr);
    QVERIFY(table != nullptr);
    QVERIFY(response != nullptr);
    url->setText(QString("http://127.0.0.1:%1/path?old=1").arg(server.serverPort()));
    QVERIFY(table->model()->setData(table->model()->index(0, 0), QStringLiteral("key")));
    QVERIFY(table->model()->setData(table->model()->index(0, 1), QStringLiteral("a b")));
    QCOMPARE(url->text(), QString("http://127.0.0.1:%1/path?key=a%20b").arg(server.serverPort()));
    QPushButton *send = nullptr;
    for (auto *button : widget.findChildren<QPushButton *>()) {
        if (button->text() == "Send") {
            send = button;
        }
    }
    QVERIFY(send != nullptr);
    send->click();
    QTRY_COMPARE_WITH_TIMEOUT(response->toPlainText(), QString("hello"), 5000);
    QVERIFY(received.startsWith("GET /path?key=a%20b HTTP/1.1\r\n"));
    bool status_found = false;
    for (const auto *label : widget.findChildren<QLabel *>()) {
        status_found |=
            label->text().contains("Status: 200 OK") && label->text().contains("Size: 5 B");
    }
    QVERIFY(status_found);
}

void TestApiTool::closeDuringRequest()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    auto *widget = new api_tool;
    auto *url = widget->findChild<QLineEdit *>();
    QVERIFY(url != nullptr);
    url->setText(QString("http://127.0.0.1:%1/").arg(server.serverPort()));
    for (auto *button : widget->findChildren<QPushButton *>()) {
        if (button->text() == "Send") {
            button->click();
            break;
        }
    }
    QTRY_VERIFY_WITH_TIMEOUT(server.hasPendingConnections(), 5000);
    delete widget;
}

QTEST_MAIN(TestApiTool)
#include "test_api_tool.moc"
