#ifndef JWT_PALETTE_H
#define JWT_PALETTE_H

/**
 * @brief JWT デコーダーの意味色
 * @details `designs/screens/jwt-decoder.pen` の変数と対応する。@n
 *          背景やテキストの基本色はアプリのテーマ (qlementine) に委ね、@n
 *          セグメント・JSON・警告・エラーの意味色だけをここで固定する。
 */
namespace JwtPalette {
/// ヘッダーセグメント (.pen: `seg-header`)
constexpr const char *SEGMENT_HEADER = "#e0245e";
/// ペイロードセグメント (.pen: `seg-payload`)
constexpr const char *SEGMENT_PAYLOAD = "#8b5cf6";
/// 署名セグメント (.pen: `seg-signature`)
constexpr const char *SEGMENT_SIGNATURE = "#1ba8d5";
/// JSON のキー (.pen: `json-key`)
constexpr const char *JSON_KEY = "#106ef9";
/// JSON の文字列値 (.pen: `json-string`)
constexpr const char *JSON_STRING = "#2bb5a0";
/// JSON の数値・真偽値・null (.pen: `json-number`)
constexpr const char *JSON_NUMBER = "#c2762c";
/// エラー文字色 (.pen: `error`)
constexpr const char *ERROR_TEXT = "#e96b72";
/// エラーバナーの背景 (.pen: Structure Error の fill)
constexpr const char *ERROR_BACKGROUND = "#f9dadc";
/// 注意バナーの背景 (.pen: `warn-bg`)
constexpr const char *NOTICE_BACKGROUND = "#feefd8";
/// 注意バナーの文字色 (.pen: `warn-fg`)
constexpr const char *NOTICE_TEXT = "#a06d10";
/// 補足テキスト (.pen: `text-secondary`)
constexpr const char *SECONDARY_TEXT = "#909090";
} // namespace JwtPalette

#endif // JWT_PALETTE_H
