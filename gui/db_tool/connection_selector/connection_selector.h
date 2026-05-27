#ifndef CONNECTION_SELECTOR_H
#define CONNECTION_SELECTOR_H

#include <QJsonObject>
#include <QSqlDatabase>
#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;

class ConnectionSelector : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionSelector(QWidget *parent = nullptr);
    ~ConnectionSelector() override = default;
    ConnectionSelector(const ConnectionSelector &) = delete;
    ConnectionSelector &operator=(const ConnectionSelector &) = delete;
    ConnectionSelector(ConnectionSelector &&) = delete;
    ConnectionSelector &operator=(ConnectionSelector &&) = delete;

    void loadHistory();

signals:
    void connectionCreated(QSqlDatabase db);
    void newConnectionRequested();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void handleHistoryItemClicked(int row);
    void handleNewConnectionButtonClicked();
    void handleCloseButtonClicked();

private:
    void buildUi();
    void retranslateUi();
    void refreshHistoryList();
    bool connectWithPassword(const QJsonObject &connectionInfo);
    void removeHistoryItem(int index);

    QLabel *titleLabel{nullptr};
    QListWidget *historyListWidget{nullptr};
    QPushButton *newConnectionButton{nullptr};
    QPushButton *closeButton{nullptr};

    QList<QJsonObject> connectionHistory;
};

#endif // CONNECTION_SELECTOR_H
