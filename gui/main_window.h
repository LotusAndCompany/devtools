#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "sidemenu.h"

#include <QMainWindow>
#include <QtGlobal>

class QAction;
class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QMimeData;
class QPushButton;
class ContentsArea;
class SettingsDialog;

/**
 * @brief メインウィンドウ
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

    /// サイドメニュー表示/非表示切り替えボタン
    QPushButton *m_sidemenuVisibilityButton{nullptr};
    /// ウィンドウを最上位に表示するトグルボタン
    QPushButton *m_windowAlwaysOnTopButton{nullptr};
    /// サイドメニュー
    Sidemenu *m_sidemenu{nullptr};
    /// コンテンツ表示領域
    ContentsArea *m_contentsArea{nullptr};

    /// メニュー: File
    QMenu *m_menuFile{nullptr};
    /// メニュー: DevTools
    QMenu *m_menuDevTools{nullptr};
    /// アクション: Settings
    QAction *m_actionSettings{nullptr};
    /// アクション: About DevTools
    QAction *m_actionAboutDevTools{nullptr};
    /// アクション: Close Window
    QAction *m_actionCloseWindow{nullptr};
    /// アクション: Show Main Window
    QAction *m_actionShowMainWindow{nullptr};

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
    /**
     * @brief ドラッグされたファイルを受け入れるか判定する
     * @param event 処理するイベント
     */
    void dragEnterEvent(QDragEnterEvent *event) override;
    /**
     * @brief ドロップされたSQLiteファイルをDBツールで開く
     * @param event 処理するイベント
     */
    void dropEvent(QDropEvent *event) override;
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
     * @brief 中央領域のウィジェットを構築する
     */
    void setupCentralWidget();
    /**
     * @brief メニューバーのアクションを構築する
     */
    void setupMenuBar();
    /**
     * @brief UIテキストを翻訳する
     */
    void retranslateUi();
    /**
     * @brief サイドメニューの表示状態を更新する
     * @param hide true で非表示
     */
    void setSidemenuHidden(bool hide);
    /**
     * @brief SQLiteファイルとして扱えるパスか判定する
     * @param filePath 対象ファイルパス
     * @return 対応するSQLiteファイルならtrue
     */
    static bool isSQLiteFilePath(const QString &filePath);
    /**
     * @brief ドロップされたURL一覧から最初のSQLiteファイルを返す
     * @param mimeData ドロップされたデータ
     * @return SQLiteファイルパス。見つからない場合は空文字列
     */
    static QString droppedSQLiteFilePath(const QMimeData *mimeData);

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
