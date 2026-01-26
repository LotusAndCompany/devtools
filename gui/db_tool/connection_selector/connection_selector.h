#ifndef CONNECTION_SELECTOR_H
#define CONNECTION_SELECTOR_H

#include <QJsonObject>
#include <QSqlDatabase>
#include <QWidget>

namespace Ui {
class ConnectionSelector;
}

class ConnectionSelector : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionSelector(QWidget *parent = nullptr);
    ~ConnectionSelector() override;

    void loadHistory();

signals:
    void connectionCreated(QSqlDatabase db);
    void newConnectionRequested();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void handleHistoryItemClicked(int row);
    void handleDeleteButtonClicked();
    void handleNewConnectionButtonClicked();
    void handleCloseButtonClicked();

private:
    Ui::ConnectionSelector *ui;
    QList<QJsonObject> connectionHistory;

    void refreshHistoryList();
    bool connectWithPassword(const QJsonObject &connectionInfo);
    void removeHistoryItem(int index);
};

#endif // CONNECTION_SELECTOR_H
