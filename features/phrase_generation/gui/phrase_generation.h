#ifndef PHRASE_GENERATION_H
#define PHRASE_GENERATION_H

#include <QWidget>

class QFrame;
class QGroupBox;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * @brief 定型文生成ツールのGUIクラス
 * @details テンプレート化された定型文の作成・管理・再利用を行うツール
 */
class phraseGeneration : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit phraseGeneration(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~phraseGeneration() override = default;
    phraseGeneration(const phraseGeneration &) = delete;
    phraseGeneration &operator=(const phraseGeneration &) = delete;
    phraseGeneration(phraseGeneration &&) = delete;
    phraseGeneration &operator=(phraseGeneration &&) = delete;

private slots:
    /**
     * @brief 追加ボタンがクリックされたときの処理
     */
    void handleAddButtonClick();
    /**
     * @brief 保存ボタンがクリックされたときの処理
     */
    void handleSaveButtonClick();
    /**
     * @brief コピーボタンがクリックされたときの処理
     */
    void handleCopyButtonClick();
    /**
     * @brief 削除ボタンがクリックされたときの処理
     */
    void handleDeleteButtonClick();
    /**
     * @brief タイトルツリーのアイテムがクリックされたときの処理
     * @param item クリックされたアイテム
     * @param column クリックされたカラム
     */
    void handleTitleTreeWidgetItemClick(QTreeWidgetItem *item, int column);

private:
    /**
     * @brief UIを構築する
     */
    void buildUi();
    /**
     * @brief ウィジェットを作成する
     */
    void createWidgets();
    /**
     * @brief ウィジェットをレイアウトする
     */
    void layoutWidgets();
    /**
     * @brief ショートカットキーを設定する
     */
    void setupShortcuts();
    /**
     * @brief UIテキストを再翻訳する
     */
    void retranslateUi();
    /**
     * @brief タイトルリストを読み込む
     */
    void loadTitles();
    /**
     * @brief ファイル名でツリーアイテムを選択する
     * @param filename 選択するファイル名
     */
    void selectTreeItemByFilename(const QString &filename);
    /**
     * @brief 未保存の変更があるかどうかを返す
     * @return 未保存の変更がある場合は `true`
     */
    [[nodiscard]] bool hasUnsavedChanges() const;
    /**
     * @brief 破棄の確認ダイアログを表示する
     * @return 破棄が確認された場合は `true`
     */
    bool confirmDiscard();
    /**
     * @brief コンテンツを読み込む
     * @param filename ファイル名
     * @param title タイトルを格納するポインタ（オプション）
     * @return 読み込まれたコンテンツ
     */
    static QString loadContent(const QString &filename, QString *title = nullptr);
    /**
     * @brief コンテンツを保存する
     * @param title タイトル
     * @param content コンテンツ
     * @return 保存されたファイル名
     */
    static QString saveContent(const QString &title, const QString &content);
    /**
     * @brief コンテンツを削除する
     * @param filename 削除するファイル名
     */
    void deleteContent(const QString &filename);

    /// テンプレートタイトル入力
    QLineEdit *template_title{nullptr};
    /// 削除ボタン
    QPushButton *delete_button{nullptr};
    /// コピーボタン
    QPushButton *copy_button{nullptr};
    /// 追加ボタン
    QPushButton *add_button{nullptr};
    /// エディタグループボックス
    QGroupBox *editor_group{nullptr};
    /// ツリーグループボックス
    QGroupBox *tree_group{nullptr};
    /// ツリーセパレータ
    QFrame *tree_separator{nullptr};
    /// エディタセパレータ
    QFrame *editor_separator{nullptr};
    /// テンプレートテキスト入力
    QPlainTextEdit *template_text{nullptr};
    /// タイトルツリーウィジェット
    QTreeWidget *title_tree_widget{nullptr};
    /// 保存ボタン
    QPushButton *save_button{nullptr};

    /// 現在編集中のファイル
    QString currentFile;

protected:
    /**
     * @brief イベント処理
     * @param event 発生したイベント
     * @details 言語変更イベントを処理する
     */
    void changeEvent(QEvent *event) override;
};

#endif // PHRASE_GENERATION_H
