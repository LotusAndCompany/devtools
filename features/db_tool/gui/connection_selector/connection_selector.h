#ifndef CONNECTION_SELECTOR_H
#define CONNECTION_SELECTOR_H

#include <QJsonObject>
#include <QSqlDatabase>
#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;

/**
 * @brief データベース接続選択ウィジェット
 * @details 過去の接続履歴から選択、または新規接続を作成する
 */
class ConnectionSelector : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit ConnectionSelector(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~ConnectionSelector() override = default;
    ConnectionSelector(const ConnectionSelector &) = delete;
    ConnectionSelector &operator=(const ConnectionSelector &) = delete;
    ConnectionSelector(ConnectionSelector &&) = delete;
    ConnectionSelector &operator=(ConnectionSelector &&) = delete;

    /**
     * @brief 接続履歴を読み込む
     */
    void loadHistory();

signals:
    /**
     * @brief データベース接続が作成されたときに発行されるシグナル
     * @param db データベース接続
     */
    void connectionCreated(QSqlDatabase db);
    /**
     * @brief 新規接続が要求されたときに発行されるシグナル
     */
    void newConnectionRequested();

protected:
    /**
     * @brief イベント処理
     * @param event 発生したイベント
     * @details 言語変更イベントを処理する
     */
    void changeEvent(QEvent *event) override;

private slots:
    /**
     * @brief 履歴アイテムがクリックされたときの処理
     * @param row クリックされた行
     */
    void handleHistoryItemClicked(int row);
    /**
     * @brief 新規接続ボタンがクリックされたときの処理
     */
    void handleNewConnectionButtonClicked();
    /**
     * @brief 閉じるボタンがクリックされたときの処理
     */
    void handleCloseButtonClicked();

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
     * @brief 履歴リストを更新する
     */
    void refreshHistoryList();
    /**
     * @brief パスワードを使って接続する
     * @param connectionInfo 接続情報
     * @return 接続成功時は `true`
     */
    bool connectWithPassword(const QJsonObject &connectionInfo);
    /**
     * @brief 履歴アイテムを削除する
     * @param index 削除するアイテムのインデックス
     */
    void removeHistoryItem(int index);

    /// タイトルラベル
    QLabel *titleLabel{nullptr};
    /// 履歴リストウィジェット
    QListWidget *historyListWidget{nullptr};
    /// 新規接続ボタン
    QPushButton *newConnectionButton{nullptr};
    /// 閉じるボタン
    QPushButton *closeButton{nullptr};

    /// 接続履歴のリスト
    QList<QJsonObject> connectionHistory;
};

#endif // CONNECTION_SELECTOR_H
