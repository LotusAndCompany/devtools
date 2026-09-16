#ifndef QUERY_PAGE_H
#define QUERY_PAGE_H

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QWidget>

class QPushButton;
class QTableView;
class QTextEdit;

/**
 * @brief SQLクエリを実行するページのウィジェット
 * @details データベースに対してSQLクエリを入力・実行し、結果を表示する
 */
class QueryPage : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit QueryPage(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~QueryPage() override = default;
    QueryPage(const QueryPage &) = delete;
    QueryPage &operator=(const QueryPage &) = delete;
    QueryPage(QueryPage &&) = delete;
    QueryPage &operator=(QueryPage &&) = delete;

protected:
    /**
     * @brief イベント処理
     * @param event 発生したイベント
     * @details 言語変更イベントを処理する
     */
    void changeEvent(QEvent *event) override;

private slots:
    /**
     * @brief クエリを実行する
     */
    void executeQuery();

private:
    /**
     * @brief UIを構築する
     */
    void buildUi();
    /**
     * @brief UIテキストを再翻訳する
     */
    void retranslateUi();

    /// クエリ入力用のテキストエディット
    QTextEdit *queryTextEdit{nullptr};
    /// 実行ボタン
    QPushButton *executeButton{nullptr};
    /// クエリ結果表示用のテーブルビュー
    QTableView *queryResultView{nullptr};

    /// データベース接続
    QSqlDatabase db;
    /// クエリ結果のモデル
    QSqlQueryModel *model{nullptr};
};

#endif // QUERY_PAGE_H
