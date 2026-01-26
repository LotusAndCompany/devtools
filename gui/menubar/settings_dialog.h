#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

/**
 * @brief メニューバーのDevTools > Settings で表示されるダイアログ
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

    /// UI
    Ui::SettingsDialog *const ui;

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit SettingsDialog(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~SettingsDialog() override;
    SettingsDialog(const SettingsDialog &) = delete;
    SettingsDialog &operator=(const SettingsDialog &) = delete;
    SettingsDialog(SettingsDialog &&) = delete;
    SettingsDialog &operator=(SettingsDialog &&) = delete;

private:
    /**
     * @brief 設定をロードする
     */
    void loadSettings();
    /**
     * @brief 設定を保存する
     */
    void saveSettings();

protected:
    /**
     * @brief ダイアログが表示される時の処理
     * @param event ショーイベント
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief イベントハンドラー（言語変更の検出用）
     * @param event イベント
     */
    void changeEvent(QEvent *event) override;

private slots:
    /**
     * @brief OKボタンが押された時の処理
     */
    void onAccepted();
    /**
     * @brief キャンセルボタンが押された時の処理
     */
    void onRejected();
    /**
     * @brief 適用ボタンが押された時の処理
     */
    void onApplyClicked();

signals:
    /**
     * @brief 言語が変更された時に発せられる
     * @param languageCode 言語コード (ja_JP, en など)
     */
    void languageChanged(const QString &languageCode);
};

#endif // SETTINGS_DIALOG_H
