#ifndef ABOUT_DEVTOOLS_DIALOG_H
#define ABOUT_DEVTOOLS_DIALOG_H

#include <QDialog>

class QWidget;

/**
 * @brief メニューバーのDevTools > About DevTools で表示されるダイアログ
 */
class AboutDevToolsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit AboutDevToolsDialog(QWidget *parent = nullptr);
    /**
     * デストラクタ
     */
    ~AboutDevToolsDialog() override = default;
    AboutDevToolsDialog(const AboutDevToolsDialog &) = delete;
    AboutDevToolsDialog &operator=(const AboutDevToolsDialog &) = delete;
    AboutDevToolsDialog(AboutDevToolsDialog &&) = delete;
    AboutDevToolsDialog &operator=(AboutDevToolsDialog &&) = delete;

private:
    /**
     * @brief About タブを構築する
     * @param parent 親ウィジェット
     * @return 構築した About タブのウィジェット
     */
    QWidget *createAboutTab(QWidget *parent);
    /**
     * @brief License タブを構築する
     * @param parent 親ウィジェット
     * @return 構築した License タブのウィジェット
     */
    static QWidget *createLicenseTab(QWidget *parent);

private slots:
    /**
     * @brief コピーボタンを押した時の処理
     */
    static void onCopyButtonClicked();
};

#endif // ABOUT_DEVTOOLS_DIALOG_H
