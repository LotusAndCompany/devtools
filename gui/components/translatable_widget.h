#ifndef GUI_COMPONENTS_TRANSLATABLE_WIDGET_H
#define GUI_COMPONENTS_TRANSLATABLE_WIDGET_H

#include <QEvent>

/**
 * @brief changeEvent + retranslateUi パターンの CRTP mixin
 *
 * GuiTool 基底クラスでの有効化と同時に、
 * QWidget を直接継承する非ツールウィジェット向けの mixin としても提供する。
 *
 * 使用方法:
 * @code
 *   class MyWidget : public TranslatableWidget<QWidget>
 *   {
 *       Q_OBJECT
 *   public:
 *       using TranslatableWidget::TranslatableWidget;
 *   protected:
 *       void changeEvent(QEvent *event) override
 *       {
 *           handleLanguageChange(event, [this] { retranslateUi(); });
 *           QWidget::changeEvent(event);
 *       }
 *       void retranslateUi() override
 *       {
 *           // re-translate UI text using tr()
 *       }
 *   };
 * @endcode
 */
template <class Base>
class TranslatableWidget : public Base
{
public:
    using Base::Base;

    virtual ~TranslatableWidget() = default;

protected:
    /**
     * @brief 言語切り替え時にサブクラスで再翻訳を実装する
     */
    virtual void retranslateUi() = 0;

    /**
     * @brief changeEvent から呼ぶヘルパー
     * @param event 処理するイベント
     * @param action LanguageChange 時に実行する処理
     */
    template <typename F>
    void handleLanguageChange(QEvent *event, F &&action)
    {
        if (event->type() == QEvent::LanguageChange) {
            action();
            event->accept();
        }
    }
};

#endif // GUI_COMPONENTS_TRANSLATABLE_WIDGET_H
