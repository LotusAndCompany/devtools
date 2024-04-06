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
#define TOOL_ID_FIELDS()        \
    /** サンプルとして入れた値その1 */   \
    SAMPLE_0,                   \
    /** サンプルとして入れた値その2 */   \
    SAMPLE_1,                   \
    /** サンプルとして入れた値その3 */   \
    SAMPLE_2,                   \
    /** サンプルとして入れた値その4 */   \
    SAMPLE_3,                   \
    /** HTTPリクエスト */        \
    HTTP_REQUEST,               \
    /** コマンド生成 */           \
    COMMAND_GENERATION,         \
    /** 画像リサイズ */           \
    IMAGE_RESIZE,               \
    /** 画像回転・反転 */          \
    IMAGE_ROTATION,             \
    /** 画像分割 */             \
    IMAGE_SPLIT,                \
    /** 画像透明化 */            \
    IMAGE_TRANSPARENT,          \
    /** 画像トリミング */          \
    IMAGE_TRIM,                 \
    /** 画像結合・重ね合わせ */   \
    IMAGE_COMBINE,              \
    /** 定型文生成 */            \
    PHRASE_GENERATION
// clang-format on

#endif // TOOL_ID_FIELDS_H
