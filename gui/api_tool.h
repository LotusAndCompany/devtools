#ifndef API_TOOL_H
#define API_TOOL_H

#include <QFrame>
#include <QStringList>

class QComboBox;
class QLabel;
class QLineEdit;
class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;
class QSplitter;
class QStandardItemModel;
class QStringListModel;
class QTableView;
class QTextEdit;

class api_tool : public QFrame
{
    Q_OBJECT

public:
    explicit api_tool(QWidget *parent = nullptr);
    ~api_tool() override;
    api_tool(const api_tool &) = delete;
    api_tool &operator=(const api_tool &) = delete;
    api_tool(api_tool &&) = delete;
    api_tool &operator=(api_tool &&) = delete;

private slots:
    void setupParametersTable();
    void handleSendButtonClick();
    void handleNetworkReplyFinished(QNetworkReply *reply);
    void updateUrlFromParams();
    void setupResponseView();

private:
    void buildUi();

    QComboBox *method_combo{};
    QTextEdit *url_edit{};
    QPushButton *send_button{};
    QTableView *params_table{};
    QLineEdit *username_edit{};
    QLineEdit *password_edit{};
    QTextEdit *body_edit{};
    QSplitter *main_splitter{};

    QNetworkAccessManager *network_manager;
    QStringList list;
    QStandardItemModel *params_model;
    QStringListModel *response_model{};
    QLabel *status_label{};
    qint64 request_start_time{};
};

#endif // API_TOOL_H
