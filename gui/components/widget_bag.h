#ifndef GUI_COMPONENTS_WIDGET_BAG_H
#define GUI_COMPONENTS_WIDGET_BAG_H

/**
 * @brief Ui::* 構造体パターンの規格化
 *
 * image_tools_unified_gui.h の Ui::ImageToolsUnifiedGUI の方式を
 * 全コンポーネントで統一するためのベースパターンを示すドキュメント用ヘッダー。
 *
 * 使用パターン:
 * @code
 * namespace Ui {
 * struct MyWidgetBag
 * {
 *     QPushButton *actionButton = nullptr;
 *     QLabel *statusLabel = nullptr;
 * };
 * } // namespace Ui
 * @endcode
 *
 * この構造体を buildUi() 関数で組み立て、コンストラクタで所有クラスに渡す。
 * 実際の実装はこのファイルには含まれず、各ツールが独自に定義する。
 */

#endif // GUI_COMPONENTS_WIDGET_BAG_H
