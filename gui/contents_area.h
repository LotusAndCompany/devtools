#ifndef CONTENTS_AREA_H
#define CONTENTS_AREA_H

#include <QFrame>
#include "sidemenu.h"

namespace Ui {
class ContentsArea;
}

/**
 * @brief メインウィンドウ右側の領域
 */
class ContentsArea : public QFrame
{
    Q_OBJECT

    Ui::ContentsArea *const ui;

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit ContentsArea(QWidget *parent = nullptr);
    /**
     * デストラクタ
     */
    ~ContentsArea();

public slots:
    /**
     * @brief サイドメニューのボタンを押した時に呼び出される
     * @param id サイドメニューID
     * @details Sidemenu::itemSelected(Sidemenu::ID id) と接続される
     * 
     * @sa MainWindow::MainWindow(QWidget *parent)
     */
    void onSidemenuItemChanged(Sidemenu::ID id);

private:
    /**
     * @brief 現在表示中のウィジェット
     */
    QWidget *currentContent = nullptr;

    /**
     * @brief イベント処理
     * @details 言語の変更があった場合にUIに反映する
     * @param event 発生したイベント
     */
    void changeEvent(QEvent *event) override;
    /**
     * @brief currentContent を指定された Sidemenu::ID の物に差し替える
     * @param id
     */
    void changeContent(Sidemenu::ID id);
};

// NOTE: 各GUIツールの実装サンプル
// TODO: 要らなくなったら消す
#include <QHBoxLayout>
#include <QLabel>
#include "core/tool/tool.h"
#include "gui_tool.h"

namespace Interface {
class SampleTool : public Tool
{
    Q_OBJECT
public:
    // NOTE: 本来はTool::ID, stringIDは渡されない
    explicit SampleTool(Tool::ID id, const QString &stringID, QObject *parent = nullptr)
        : Tool(id, stringID, parent)
    {}

    virtual ~SampleTool() = default;

    virtual void f() = 0;
    virtual void g() = 0;
    virtual void h() = 0;
};
} // namespace Interface

class SampleTool : public Interface::SampleTool
{
    Q_OBJECT

public:
    // NOTE: 本来はTool::ID, stringIDは渡されない
    explicit SampleTool(Tool::ID id, const QString &stringID, QObject *parent = nullptr)
        : Interface::SampleTool(id, stringID, parent)
    {}
    ~SampleTool() { qDebug() << "~SampleTool()"; }

    void f() override {}
    void g() override {}
    void h() override {}
};

class SampleGuiTool : public GuiTool
{
    Q_OBJECT

public:
    // NOTE: 本来はTool::ID, stringIDは渡さずToolクラスのコンストラクタ内で設定する
    explicit SampleGuiTool(Tool::ID id, const QString &stringID, QWidget *parent = nullptr)
        : GuiTool(parent)
        , tool(new SampleTool(id, stringID, this))
    {
        QHBoxLayout *const hLayout = new QHBoxLayout(this);

        QLabel *const label = new QLabel(tool->translatable().name, this);
        label->setAlignment(Qt::AlignCenter);
        hLayout->addWidget(label);
        setLayout(hLayout);

        tool->f();
        tool->g();
        tool->h();
    }
    ~SampleGuiTool() { qDebug() << "~SampleGuiTool()"; }

private:
    // NOTE: 基底クラス型のポインタを持っていても不便なので派生クラス型のポインタを持つ
    Interface::SampleTool *const tool;

    // NOTE: 本来は翻訳の適用等の適切な処理をする
    void changeEvent(QEvent *event) override { return QWidget::changeEvent(event); }
};

#endif // CONTENTS_AREA_H
