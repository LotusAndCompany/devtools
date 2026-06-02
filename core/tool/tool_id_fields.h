#ifndef TOOL_ID_FIELDS_H
#define TOOL_ID_FIELDS_H

/**
 * @file
 * @brief TOOL_ID_FIELDS()を定義している
 */

/**
 * @def TOOL_ID_FIELDS()
 * @brief Tool::ID 及び Sidemenu::ID に展開する定数名
 */
// NOTE: コード整形をこの部分だけ無効化する
// clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TOOL_ID_FIELDS()        \
    /** HTTPリクエスト */        \
    HTTP_REQUEST,               \
    /** コマンド生成 */           \
    COMMAND_GENERATION,         \
    /** 画像統合編集 */           \
    IMAGE_ALL_IN_ONE,           \
    /** 画像トリミング */         \
    IMAGE_TRIM,                 \
    /** 画像結合・重ね合わせ */    \
    IMAGE_COMBINE,              \
    /** 定型文生成 */             \
    PHRASE_GENERATION,          \
    /** データ・フォーマット変換 */ \
    DATA_CONVERSION,             \
    /** DBツール */              \
    DB_TOOL,                     \
    /** QRコード生成 */           \
    QR_CODE_GENERATION,          \
    /** 正規表現テスター */        \
    REGEX_TESTER                  // 末尾のカンマは不要
// clang-format on

#endif // TOOL_ID_FIELDS_H
