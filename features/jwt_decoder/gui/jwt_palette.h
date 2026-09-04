#ifndef JWT_PALETTE_H
#define JWT_PALETTE_H

#include <QColor>

/**
 * @brief JWT デコーダーの意味色
 * @details `designs/screens/jwt-decoder.pen` の変数と対応する。@n
 *          `.pen` のパレットは qlementine の Light テーマからの引き写しのため、@n
 *          テーマに対応するキーがある色はテーマから取得し Light / Dark 双方に追従させる。@n
 *          既定値は Light テーマの値で、テーマを取得できない場合のフォールバックを兼ねる。
 */
struct JwtPalette
{
    /// ヘッダーセグメント (.pen: `seg-header`) — テーマに対応するキーなし
    QColor segmentHeader{QColor::fromRgb(0xe0245e)};
    /// ペイロードセグメント (.pen: `seg-payload`) — テーマに対応するキーなし
    QColor segmentPayload{QColor::fromRgb(0x8b5cf6)};
    /// 署名セグメント (.pen: `seg-signature`) ← `statusColorInfo`
    QColor segmentSignature{QColor::fromRgb(0x1ba8d5)};
    /// JSON のキー (.pen: `json-key`) ← `primaryAlternativeColor`
    QColor jsonKey{QColor::fromRgb(0x106ef9)};
    /// JSON の文字列値 (.pen: `json-string`) ← `statusColorSuccess`
    QColor jsonString{QColor::fromRgb(0x2bb5a0)};
    /// JSON の数値・真偽値・null (.pen: `json-number`) — テーマに対応するキーなし
    QColor jsonNumber{QColor::fromRgb(0xc2762c)};
    /// エラー文字色 (.pen: `error`) ← `statusColorError`
    QColor errorText{QColor::fromRgb(0xe96b72)};
    /// エラーバナーの背景 (.pen: Structure Error の fill) ← `statusColorErrorDisabled`
    QColor errorBackground{QColor::fromRgb(0xf9dadc)};
    /// 注意バナーの背景 (.pen: `warn-bg`) ← `statusColorWarningDisabled`
    QColor noticeBackground{QColor::fromRgb(0xfeefd8)};
    /// 注意バナーの文字色 ← `secondaryColor` (注意の色味は背景で表現する)
    QColor noticeText{QColor::fromRgb(0x404040)};
    /// 補足テキスト (.pen: `text-secondary`) ← `secondaryAlternativeColor`
    QColor secondaryText{QColor::fromRgb(0x909090)};

    /**
     * @brief 現在の qlementine テーマから構築する
     * @details スタイルが `QlementineStyle` でない場合 (素の QApplication を使う
     *          テスト等) は既定値をそのまま返す。
     */
    [[nodiscard]] static JwtPalette current();

    [[nodiscard]] bool operator==(const JwtPalette &other) const;
    [[nodiscard]] bool operator!=(const JwtPalette &other) const { return !(*this == other); }
};

#endif // JWT_PALETTE_H
