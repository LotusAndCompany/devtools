#ifndef CONTENTS_AREA_H
#define CONTENTS_AREA_H

#include "sidemenu.h"

#include <QFrame>

class QVBoxLayout;
class dbMain;

/**
 * @brief メインウィンドウ右側の領域
 */
class ContentsArea : public QFrame
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit ContentsArea(QWidget *parent = nullptr);
    /**
     * デストラクタ
     */
    ~ContentsArea() override = default;
    ContentsArea(const ContentsArea &) = delete;
    ContentsArea &operator=(const ContentsArea &) = delete;
    ContentsArea(ContentsArea &&) = delete;
    ContentsArea &operator=(ContentsArea &&) = delete;

    /**
     * @brief サイドメニューのボタンを押した時に呼び出される
     * @param id サイドメニューID
     * @details Sidemenu::itemSelected(Sidemenu::ID id) と接続される
     *
     * @sa MainWindow::MainWindow(QWidget *parent)
     */
    void onSidemenuItemChanged(Sidemenu::ID id);

public slots:
    /**
     * @brief SQLiteファイルをDBツールで開く
     * @param filePath SQLiteファイルパス
     * @return 接続できた場合はtrue
     */
    bool openSQLiteFileInDbTool(const QString &filePath);

private:
    /// レイアウト
    QVBoxLayout *m_contentsAreaLayout{nullptr};

    /**
     * @brief 現在表示中のウィジェット
     */
    QWidget *currentContent = nullptr;

    /**
     * @brief DBツールのキャッシュ（状態を保持するため）
     */
    dbMain *cachedDbMain = nullptr;

    /**
     * @brief currentContent を指定された Sidemenu::ID の物に差し替える
     * @param id
     */
    void changeContent(Sidemenu::ID id);
};
#endif // CONTENTS_AREA_H
