#ifndef WELCOME_PAGE_H
#define WELCOME_PAGE_H

#include <QWidget>

class QLabel;

/**
 * @brief ウェルカムページのウィジェット
 * @details アプリケーション起動時に表示される初期画面
 */
class WelcomePage : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit WelcomePage(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~WelcomePage() override = default;
    WelcomePage(const WelcomePage &) = delete;
    WelcomePage &operator=(const WelcomePage &) = delete;
    WelcomePage(WelcomePage &&) = delete;
    WelcomePage &operator=(WelcomePage &&) = delete;

protected:
    /**
     * @brief イベント処理
     * @param event 発生したイベント
     * @details 言語変更イベントを処理する
     */
    void changeEvent(QEvent *event) override;

private:
    /// メッセージ表示用のラベル
    QLabel *m_messageLabel{nullptr};

    /**
     * @brief UIテキストを再翻訳する
     */
    void retranslateUi();
};

#endif // WELCOME_PAGE_H
