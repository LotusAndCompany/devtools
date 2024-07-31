#ifndef API_TOOL_H
#define API_TOOL_H

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListView>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QDateTime>
#include "customsplitter.h"

namespace Ui {
class api_tool;
}

class api_tool : public QFrame
{
    Q_OBJECT
    QWidget *currentContent = nullptr;
    CustomSplitter *mainSplitter;

public:
    explicit api_tool(QWidget *parent = nullptr);
    ~api_tool();

private slots:
    void setupParametersTable();
    void setupBodyTab();
    void setupAuthenticationTab();
    void setupMainSplitter();
    void handleSendButtonClick();  // sendButton
    void handleNetworkReplyFinished(QNetworkReply *reply);
    void updateUrlFromParams(); // パラメータ
    void setupResponseView();

private:
    Ui::api_tool *ui;
    QNetworkAccessManager *networkManager;  // ネットワークマネージャの追加
    QStringList list; // リストデータの追加
    QStandardItemModel *paramsModel;
    QStringListModel *responseModel;
    QLabel *statusLabel;
    QVBoxLayout *responseLayout;
    qint64 requestStartTime;
    QLineEdit *usernameEdit;  // ユーザー名入力欄
    QLineEdit *passwordEdit;  // パスワード入力欄
    QTextEdit *bodyTextEdit;  // ボディテキストエリア
};

#endif // API_TOOL_H
