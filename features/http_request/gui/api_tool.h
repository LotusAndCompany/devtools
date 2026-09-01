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

/**
 * @brief HTTPリクエストを送信するAPIテストツール
 * @details REST APIのテストやデバッグに使用するGUIツール
 */
class api_tool : public QFrame
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit api_tool(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~api_tool() override;
    api_tool(const api_tool &) = delete;
    api_tool &operator=(const api_tool &) = delete;
    api_tool(api_tool &&) = delete;
    api_tool &operator=(api_tool &&) = delete;

private slots:
    /**
     * @brief パラメータテーブルを設定する
     */
    void setupParametersTable();
    /**
     * @brief 送信ボタンがクリックされたときの処理
     */
    void handleSendButtonClick();
    /**
     * @brief ネットワークリプライが完了したときの処理
     * @param reply ネットワークリプライ
     */
    void handleNetworkReplyFinished(QNetworkReply *reply);
    /**
     * @brief パラメータからURLを更新する
     */
    void updateUrlFromParams();
    /**
     * @brief レスポンスビューを設定する
     */
    void setupResponseView();

private:
    /**
     * @brief UIを構築する
     */
    void buildUi();

    /// HTTPメソッド選択用のコンボボックス
    QComboBox *method_combo{};
    /// URL入力用のテキストエディット
    QTextEdit *url_edit{};
    /// 送信ボタン
    QPushButton *send_button{};
    /// パラメータ入力用のテーブルビュー
    QTableView *params_table{};
    /// 認証用のユーザー名入力
    QLineEdit *username_edit{};
    /// 認証用のパスワード入力
    QLineEdit *password_edit{};
    /// リクエストボディ入力用のテキストエディット
    QTextEdit *body_edit{};
    /// メインスプリッター
    QSplitter *main_splitter{};

    /// ネットワークアクセスマネージャ
    QNetworkAccessManager *network_manager;
    /// 文字列リスト
    QStringList list;
    /// パラメータテーブルのモデル
    QStandardItemModel *params_model;
    /// レスポンス表示用のモデル
    QStringListModel *response_model{};
    /// ステータス表示用のラベル
    QLabel *status_label{};
    /// リクエスト開始時刻
    qint64 request_start_time{};
};

#endif // API_TOOL_H
