#ifndef GUI_APPLICATION_H
#define GUI_APPLICATION_H

#include "core/application_mixin.h"
#include "main_window.h"

#include <QApplication>
#include <QtSystemDetection>

#include <memory>

/**
 * @brief GUIアプリケーション
 */
class GuiApplication : public QApplication, public ApplicationMixin
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @details シグナルを接続している@n
     *          MainWindow::colorSchemeChanged() →
     * GuiApplication::onWindowColorSchemeChanged()
     * @param argc `int main(int argc, char *argv[])`で受け取った `argc`
     * @param argv `int main(int argc, char *argv[])`で受け取った `argv`
     */
    GuiApplication(int argc, char **argv);
    GuiApplication() = delete;

    /**
     * @brief 現在の言語
     * @return 現在の言語。言語に依るかも知れないが、ja_JPのような形式。
     */
    inline QString language() { return translator().language(); }

    /**
     * @brief 言語を変更する
     * @param languageCode 新しい言語コード（ja_JP, en など）
     * @return 変更に成功したかどうか
     */
    bool changeLanguage(const QString &languageCode);

    /**
     * @brief システムのカラースキームを適用する
     */
    void applySystemColorScheme();

    void setup() override;
    int start() override;

private:
    /**
     * @brief メインウィンドウ
     */
    std::unique_ptr<MainWindow> window;

    // NOTE: staticにできる
    /**
     * @brief カラースキームを変更をUIに反映する
     * @details ライトモードの時には黒いアイコン(res/light/)@n
     *          ダークモードの時は白いアイコン(res/dark/)
     * を使うようにアイコンテーマを変更する@n 各アイコンテーマは
     * res/light/index.theme, res/dark/index.theme で定義されている
     *
     * @sa [QIcon::setThemeName(const QString
     * &name)](https://doc.qt.io/qt-6/qicon.html#setThemeName)
     * @sa [Freedesktop The icon theme
     * specification](https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html)
     */
    void applyColorScheme();

private slots:
    // NOTE: staticにできる
    /**
     * @brief カラースキームが変更された時に呼び出される
     * @details MainWindow::colorSchemeChanged() と接続される
     *
     * @sa GuiApplication::GuiApplication(int argc, char **argv)
     */
    void onWindowColorSchemeChanged();

// Platform specific
#ifdef Q_OS_MACOS
private:
    /**
     * @brief イベント処理
     * @details
     * MacOSの場合、Dockのアイコンがクリックされた時にウィンドウを表示する
     * @return 処理した場合は`true`、無視した場合は`false`
     *
     * @sa MainWindow::closeEvent(QCloseEvent *event)
     */
    bool event(QEvent *) override;
/*
private slots:
    void onApplicationStateChanged(Qt::ApplicationState state);
*/
#endif
};

#endif // GUI_APPLICATION_H
