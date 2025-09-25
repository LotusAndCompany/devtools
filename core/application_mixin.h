#ifndef APPLICATION_MIXIN_H
#define APPLICATION_MIXIN_H

/**
 * @file
 * @brief GUI/CLIアプリケーションに共通で追加する機能
 */

#include <QCoreApplication>
#include <QTranslator>

/**
 * @brief アプリケーションの種類
 */
enum class AppType {
    MIN, ///< 最小値
    GUI, ///< GUIアプリケーション
    CLI, ///< CLIアプリケーション
    MAX, ///< 最大値
};

/**
 * @brief [QCoreApplication](https://doc.qt.io/qt-6/qcoreapplication.html)クラスにmixinとして追加する機能
 */
class ApplicationMixin
{
public:
    /**
     * @brief コンストラクタ
     * @param appType GUIアプリケーションならAppType::GUI、CLIアプリケーションならAppType::CLIを渡す
     * @param argc `int main(int argc, char *argv[])`で受け取った `argc`
     * @param argv `int main(int argc, char *argv[])`で受け取った `argv`
     */
    ApplicationMixin(AppType appType, int argc, char **argv);
    ApplicationMixin() = delete;

    /// アプリケーションの種類
    const AppType appType;
    /**
     * @brief アプリケーションの種類の判定
     * @return GUIアプリケーションであれば `true`
     */
    inline bool isGui() const { return appType == AppType::GUI; }
    /**
     * @brief アプリケーションの種類の判定
     * @return CLIアプリケーションであれば `true`
     */
    inline bool isCli() const { return appType == AppType::CLI; }

    /**
     * @brief GUI/CLIに固有の初期化処理
     */
    virtual void setup() = 0;
    /**
     * @brief アプリケーションの処理開始
     * @return ステータス(0=正常終了)
     * @details GUIアプリケーションの時は[QApplication::exec()](https://doc.qt.io/qt-6/qapplication.html#exec)の結果@n
     *          CLIアプリケーションの時は[QCoreApplication::exec()](https://doc.qt.io/qt-6/qcoreapplication.html#exec)の結果
     *          を返す
     */
    virtual int start() = 0;

protected:
    /**
     * @brief GUI/CLIに共通の初期化処理
     * @param app 初期化するアプリケーションへのポインタ。mixinとして使う想定なのでthisで良い。
     */
    void setupApplication(QCoreApplication *app);
    /**
     * @brief 現在有効な[QTranslator](https://doc.qt.io/qt-6/qtranslator.html)への参照を返す
     * @return QTranslatorの参照
     */
    inline const QTranslator &translator() const { return _translator; }
    
    /**
     * @brief 翻訳器への非const参照を返す（サブクラス専用）
     * @return QTranslatorの非const参照
     */
    inline QTranslator &mutableTranslator() { return _translator; }

private:
    /// 翻訳ファイルを読み込むオブジェクト
    QTranslator _translator;
};

#endif // APPLICATION_MIXIN_H
