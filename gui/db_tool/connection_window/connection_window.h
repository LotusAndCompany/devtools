#ifndef CONNECTION_WINDOW_H
#define CONNECTION_WINDOW_H

#include <QJsonObject>
#include <QSqlDatabase>
#include <QWidget>

namespace Ui {
class ConnectionWindow;
}

class ConnectionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectionWindow(QWidget *parent = nullptr);
    ~ConnectionWindow() override;
    ConnectionWindow(const ConnectionWindow &) = delete;
    ConnectionWindow &operator=(const ConnectionWindow &) = delete;
    ConnectionWindow(ConnectionWindow &&) = delete;
    ConnectionWindow &operator=(ConnectionWindow &&) = delete;

    [[nodiscard]] QJsonObject getConnectionInfo() const;

private:
    Ui::ConnectionWindow *ui;
    QJsonObject lastConnectionInfo;

    void init();
    void selectedDBType();
    void createNewConnect();
    void browseForDatabase();

signals:
    void connectionCreated(QSqlDatabase db, QJsonObject connectionInfo);

protected:
    void changeEvent(QEvent *event) override;
};

#endif // CONNECTION_WINDOW_H
