#ifndef API_TOOL_H
#define API_TOOL_H

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QStringList>
#include <QWidget>

namespace Ui {
class api_tool;
}

class QNetworkAccessManager;
class QNetworkReply;
class QStandardItemModel;
class QStringListModel;
class QVBoxLayout;
class QTextEdit;

class api_tool : public QFrame
{
    Q_OBJECT
    QWidget *currentContent = nullptr;

public:
    explicit api_tool(QWidget *parent = nullptr);
    ~api_tool();

private slots:
    void setupParametersTable();
    void handleSendButtonClick(); // sendButton
    void handleNetworkReplyFinished(QNetworkReply *reply);
    void updateUrlFromParams(); // パラメータ
    void setupResponseView();

private:
    Ui::api_tool *ui;
    QNetworkAccessManager *networkManager; // ネットワークマネージャの追加
    QStringList list;                      // リストデータの追加
    QStandardItemModel *paramsModel;
    QStringListModel *responseModel;
    QLabel *statusLabel;
    QVBoxLayout *responseLayout;
    qint64 requestStartTime;
    QLineEdit *usernameEdit; // ユーザー名入力欄
    QLineEdit *passwordEdit; // パスワード入力欄
    QTextEdit *bodyTextEdit; // ボディテキストエリア
};

#endif // API_TOOL_H
