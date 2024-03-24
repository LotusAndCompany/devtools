#ifndef SIDEMENU_ITEM_H
#define SIDEMENU_ITEM_H

#include <QPushButton>
#include "gui/sidemenu.h"

// TODO: ツールの説明文を表示しようとすると機能が足りないのでQWidgetに変える
//       uiファイルも作る
//       uiファイル側にidの情報を設定するのは難しそう?
class SidemenuItem : public QPushButton
{
    Q_OBJECT

    Sidemenu::ItemID id = Sidemenu::ItemID::UNDEFINED;

public:
    explicit SidemenuItem(QWidget *parent = nullptr);
    SidemenuItem(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
    virtual ~SidemenuItem() = default;

    // TODO: 良いデザインパターンがあれば書き直す
    // FIXME: ホーム画面でツール一覧を表示する時にも使うのでここに置くべきではない
    struct UiData
    {
        QIcon icon;
        QString text;

        static QMap<Sidemenu::ItemID, UiData> list;
        static void initializeIcon();
        static void initializeText();
        static bool isListValid();

    private:
        static QString currentLanguage;
    };

    void configure(Sidemenu::ItemID id);

private:
    void changeEvent(QEvent *event) override;
    inline bool isConfigured() const { return id != Sidemenu::ItemID::UNDEFINED; }
};

#endif // SIDEMENU_ITEM_H
