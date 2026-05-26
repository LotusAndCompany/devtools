#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QEvent;
class QGroupBox;
class QLabel;
class QShowEvent;
class QTabWidget;
class QWidget;

/**
 * @brief メニューバーのDevTools > Settings で表示されるダイアログ
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

    /// タブウィジェット
    QTabWidget *tab_widget = nullptr;
    /// General タブ
    QWidget *general_tab = nullptr;
    /// Window タブ
    QWidget *window_tab = nullptr;
    /// 言語ラベル
    QLabel *language_label = nullptr;
    /// 言語コンボボックス
    QComboBox *language_combo_box = nullptr;
    /// 起動時にサイドバーを表示するチェックボックス
    QCheckBox *show_sidebar_on_startup_check_box = nullptr;
    /// 起動時に前回使用した機能を表示するチェックボックス
    QCheckBox *show_last_tool_on_startup_check_box = nullptr;
    /// ウィンドウ動作 GroupBox
    QGroupBox *window_behavior_group_box = nullptr;
    /// Always on top チェックボックス
    QCheckBox *always_on_top_check_box = nullptr;
    /// ウィンドウサイズを記憶するチェックボックス
    QCheckBox *remember_window_size_check_box = nullptr;
    /// ウィンドウ位置を記憶するチェックボックス
    QCheckBox *remember_window_position_check_box = nullptr;
    /// ダイアログボタンボックス
    QDialogButtonBox *button_box = nullptr;

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit SettingsDialog(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~SettingsDialog() override = default;
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
    /**
     * @brief 翻訳可能なテキストを再設定する
     */
    void retranslateUi();
    /**
     * @brief General タブを構築する
     */
    void buildGeneralTab();
    /**
     * @brief Window タブを構築する
     */
    void buildWindowTab();

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
