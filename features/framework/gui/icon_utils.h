#ifndef ICON_UTILS_H
#define ICON_UTILS_H

#include <QIcon>
#include <QString>
#include <QStyle>

namespace IconUtils {
/**
 * @brief Material Symbols Outlined フォントをアプリケーションへ登録する
 * @return フォントの登録とファミリー名の検証に成功した場合は `true`
 */
bool initializeMaterialSymbols();

/**
 * @brief Material Symbols のテーマを再適用する
 * @details QIcon の既存エンジンを新しいパレットで再描画させるため、テーマキーを更新する
 */
void refreshMaterialSymbolsTheme();

/**
 * @brief テーマアイコンを取得する
 * @param name Material Symbols のグリフ名
 * @param fallback テーマアイコンを取得できない場合に使う標準アイコン
 * @return テーマアイコン、または標準アイコン
 */
QIcon themedIcon(const QString &name, QStyle::StandardPixmap fallback = QStyle::SP_FileIcon);
} // namespace IconUtils

#endif // ICON_UTILS_H
