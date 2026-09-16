#ifndef COMMAND_H
#define COMMAND_H

#include <QGroupBox>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTextBrowser;

/**
 * @brief コマンド生成ツールのGUIクラス
 * @details コマンドラインコマンドを対話的に生成するためのUIを提供する
 */
class Command : public QGroupBox
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit Command(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~Command() override = default;
    Command(const Command &) = delete;
    Command &operator=(const Command &) = delete;
    Command(Command &&) = delete;
    Command &operator=(Command &&) = delete;

private:
    /**
     * @brief UIを構築する
     */
    void buildUi();
    /**
     * @brief 初期化処理を実行する
     */
    void init();
    /**
     * @brief カテゴリが選択されたときの処理
     */
    void selectedCategory();
    /**
     * @brief 機能が選択されたときの処理
     */
    void selectedFunction();
    /**
     * @brief オプションが選択されたときの処理
     */
    void selectedOption();
    /**
     * @brief コマンドを生成する
     */
    void generate();
    /**
     * @brief 入力をリセットする
     */
    void reset();
    /**
     * @brief 出力をクリアする
     */
    void clear();
    /**
     * @brief 生成されたコマンドをクリップボードにコピーする
     */
    void copy();
    /**
     * @brief コマンドボックスの幅を調整する
     */
    void adjustCommandBoxWidth();

    /// カテゴリ選択用のコンボボックス
    QComboBox *category_list{nullptr};
    /// 機能リストのラベル
    QLabel *functions_label{nullptr};
    /// 機能選択用のコンボボックス
    QComboBox *functions_list{nullptr};
    /// オプションリストのラベル
    QLabel *option_label{nullptr};
    /// オプション選択用のコンボボックス
    QComboBox *option_list{nullptr};
    /// テキスト入力のラベル
    QLabel *text_label{nullptr};
    /// テキスト入力用のエディット
    QLineEdit *text_edit{nullptr};
    /// リセットボタン
    QPushButton *reset_button{nullptr};
    /// 生成ボタン
    QPushButton *generate_button{nullptr};
    /// 生成されたコマンドを表示するブラウザ
    QTextBrowser *text_browser{nullptr};
    /// コピーボタン
    QPushButton *copy_button{nullptr};
    /// クリアボタン
    QPushButton *clear_button{nullptr};
};

#endif // COMMAND_H
