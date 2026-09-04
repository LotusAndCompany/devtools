#ifndef CONNECTION_WINDOW_H
#define CONNECTION_WINDOW_H

#include <QJsonObject>
#include <QSqlDatabase>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

/**
 * @brief データベース接続設定ウィンドウ
 * @details データベースへの接続情報を入力し、接続を確立するためのダイアログ
 */
class ConnectionWindow : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit ConnectionWindow(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~ConnectionWindow() override = default;
    ConnectionWindow(const ConnectionWindow &) = delete;
    ConnectionWindow &operator=(const ConnectionWindow &) = delete;
    ConnectionWindow(ConnectionWindow &&) = delete;
    ConnectionWindow &operator=(ConnectionWindow &&) = delete;

    /**
     * @brief 接続情報を取得する
     * @return 接続情報を含むJSONオブジェクト
     */
    [[nodiscard]] QJsonObject getConnectionInfo() const;

signals:
    /**
     * @brief データベース接続が作成されたときに発行されるシグナル
     * @param db データベース接続
     * @param connectionInfo 接続情報
     */
    void connectionCreated(QSqlDatabase db, QJsonObject connectionInfo);

protected:
    /**
     * @brief イベント処理
     * @param event 発生したイベント
     * @details 言語変更イベントを処理する
     */
    void changeEvent(QEvent *event) override;
    /**
     * @brief イベントフィルタ
     * @param watched 監視対象のオブジェクト
     * @param event 発生したイベント
     * @return イベントを処理した場合は `true`
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    /**
     * @brief UIを構築する
     */
    void buildUi();
    /**
     * @brief UIテキストを再翻訳する
     */
    void retranslateUi();
    /**
     * @brief 初期化処理を実行する
     */
    void init();
    /**
     * @brief データベースタイプが選択されたときの処理
     */
    void selectedDBType();
    /**
     * @brief 新しい接続を作成する
     */
    void createNewConnect();
    /**
     * @brief データベースファイルを参照する
     */
    void browseForDatabase();

    /// データベースタイプのラベル
    QLabel *dbTypeLabel{nullptr};
    /// データベースタイプ選択用のコンボボックス
    QComboBox *dbTypeComboBox{nullptr};
    /// ホスト名のラベル
    QLabel *hostNameLabel{nullptr};
    /// ホスト名入力用のラインエディット
    QLineEdit *hostNameLineEdit{nullptr};
    /// データベース名のラベル
    QLabel *dbNameLabel{nullptr};
    /// データベース名入力用のラインエディット
    QLineEdit *dbNamelineEdit{nullptr};
    /// 参照ボタン
    QPushButton *browseButton{nullptr};
    /// ユーザー名のラベル
    QLabel *userNameLabel{nullptr};
    /// ユーザー名入力用のラインエディット
    QLineEdit *userNameLineEdit{nullptr};
    /// パスワードのラベル
    QLabel *passwordLabel{nullptr};
    /// パスワード入力用のラインエディット
    QLineEdit *passwordLineEdit{nullptr};
    /// 接続ボタン
    QPushButton *ConnectPushButton{nullptr};
    /// 閉じるボタン
    QPushButton *ClosePushButton{nullptr};

    /// 最後の接続情報
    QJsonObject lastConnectionInfo;
};

#endif // CONNECTION_WINDOW_H
