#ifndef CONNECTION_WINDOW_H
#define CONNECTION_WINDOW_H

#include <QJsonObject>
#include <QSqlDatabase>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

class ConnectionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectionWindow(QWidget *parent = nullptr);
    ~ConnectionWindow() override = default;
    ConnectionWindow(const ConnectionWindow &) = delete;
    ConnectionWindow &operator=(const ConnectionWindow &) = delete;
    ConnectionWindow(ConnectionWindow &&) = delete;
    ConnectionWindow &operator=(ConnectionWindow &&) = delete;

    [[nodiscard]] QJsonObject getConnectionInfo() const;

signals:
    void connectionCreated(QSqlDatabase db, QJsonObject connectionInfo);

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void buildUi();
    void retranslateUi();
    void init();
    void selectedDBType();
    void createNewConnect();
    void browseForDatabase();

    QLabel *dbTypeLabel{nullptr};
    QComboBox *dbTypeComboBox{nullptr};
    QLabel *hostNameLabel{nullptr};
    QLineEdit *hostNameLineEdit{nullptr};
    QLabel *dbNameLabel{nullptr};
    QLineEdit *dbNamelineEdit{nullptr};
    QPushButton *browseButton{nullptr};
    QLabel *userNameLabel{nullptr};
    QLineEdit *userNameLineEdit{nullptr};
    QLabel *passwordLabel{nullptr};
    QLineEdit *passwordLineEdit{nullptr};
    QPushButton *ConnectPushButton{nullptr};
    QPushButton *ClosePushButton{nullptr};

    QJsonObject lastConnectionInfo;
};

#endif // CONNECTION_WINDOW_H
