#ifndef ABOUT_DEVTOOLS_DIALOG_H
#define ABOUT_DEVTOOLS_DIALOG_H

#include <QDialog>

namespace Ui {
class AboutDevToolsDialog;
}

/**
 * @brief メニューバーのDevTools > About DevTools で表示されるダイアログ
 */
class AboutDevToolsDialog : public QDialog
{
    Q_OBJECT

    /// UI
    Ui::AboutDevToolsDialog *const ui;

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit AboutDevToolsDialog(QWidget *parent = nullptr);
    /**
     * デストラクタ
     */
    ~AboutDevToolsDialog() override;
    AboutDevToolsDialog(const AboutDevToolsDialog &) = delete;
    AboutDevToolsDialog &operator=(const AboutDevToolsDialog &) = delete;
    AboutDevToolsDialog(AboutDevToolsDialog &&) = delete;
    AboutDevToolsDialog &operator=(AboutDevToolsDialog &&) = delete;

private slots:
    /**
     * @brief コピーボタンを押した時の処理
     */
    static void onCopyButtonClicked();
};

#endif // ABOUT_DEVTOOLS_DIALOG_H
