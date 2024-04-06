#ifndef TOOL_ID_FIELDS_H
#define TOOL_ID_FIELDS_H

/**
 * @file
 * @brief TOOL_ID_FIELDS()を定義している
 */

/**
 * @def TOOL_ID_FIELDS()
 * @brief Tool::ID及びSidemenu::IDに展開する要素
 */
// NOTE: コード整形をこの部分だけ無効化する
// clang-format off
#define TOOL_ID_FIELDS()    \
    SAMPLE_0,               \
    SAMPLE_1,               \
    SAMPLE_2,               \
    SAMPLE_3,               \
    HTTP_REQUEST,           \
    COMMAND_GENERATION,     \
    IMAGE_RESIZE,           \
    IMAGE_ROTATION,         \
    IMAGE_SPLIT,            \
    IMAGE_TRANSPARENT,      \
    IMAGE_TRIM,             \
    IMAGE_COMBINE,          \
    PHRASE_GENERATION
// clang-format on

#endif // TOOL_ID_FIELDS_H
