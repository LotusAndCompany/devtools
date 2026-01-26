#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "sidemenu.h"

#include <QMainWindow>
#include <QtSystemDetection>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class SettingsDialog;

/**
 * @brief メインウィンドウ
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

    /// UI
    Ui::MainWindow *const ui;

    /// 設定ダイアログ
    SettingsDialog *m_settingsDialog{nullptr};

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     * @details シグナルの接続をしている@n
     *          QMenuBar::triggered(QAction *action) → ::onActionTriggered(QAction
     * *action)@n Sidemenu::itemSelected(Sidemenu::ID id) →
     * ContentsArea::onSidemenuItemChanged(Sidemenu::ID id)@n
     *          QPushButton::clicked() → ::onSidemenuVisibilityButtonClicked()@n
     *          QPushButton::toggled(bool checked) →
     * ::onWindowAlwaysOnTopButtonToggled(bool checked)
     */
    MainWindow(QWidget *parent = nullptr);
    /**
     * @brief デストラクタ
     */
    ~MainWindow() override;
    MainWindow(const MainWindow &) = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    MainWindow(MainWindow &&) = delete;
    MainWindow &operator=(MainWindow &&) = delete;

private:
    /**
     * @brief 変更系のイベント処理
     * @param event 処理するイベント
     */
    void changeEvent(QEvent *event) override;
#ifdef Q_OS_MACOS
    /**
     * @brief ウィンドウを閉じる時のイベント
     * @param event 処理するイベント
     * @details MacOSの場合は×ボタンが押された時にウィンドウを隠す@n
     *          ウィンドウを表示する処理は GuiApplication::event(QEvent *event)
     * で実装
     */
    void closeEvent(QCloseEvent *event) override;
#endif

    /**
     * @brief サイドメニューの表示状態を更新する
     * @param hide true で非表示
     */
    void setSidemenuHidden(bool hide);

signals:
    /**
     * @brief カラースキームが変更された時に発せられる
     * @details GuiApplication::onWindowColorSchemeChanged() に接続される
     *
     * @sa GuiApplication::GuiApplication(int argc, char **argv)
     */
    void colorSchemeChanged();

private slots:
    /**
     * @brief メニューバーの各要素が選択された時の処理
     * @param action 選択されたアクション
     */
    void onActionTriggered(QAction *action);
    /**
     * @brief サイドメニュー表示/非表示切り替えボタンが押された時の処理
     */
    void onSidemenuVisibilityButtonClicked();
    /**
     * @brief ウィンドウを最上位に表示するボタンが切り替わった時の処理
     * @param checked ウィンドウを最上位に表示するボタンの状態
     */
    void onWindowAlwaysOnTopButtonToggled(bool checked);
    /**
     * @brief 言語が変更された時の処理
     * @param languageCode 言語コード（ja_JP, en など）
     */
    void onLanguageChanged(const QString &languageCode);
    /**
     * @brief サイドメニューの選択が変わった時の処理
     * @param id 選択されたサイドメニューID
     */
    static void onSidemenuItemSelected(Sidemenu::ID id);
};
#endif // MAIN_WINDOW_H
