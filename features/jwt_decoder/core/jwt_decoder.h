#ifndef JWT_DECODER_H
#define JWT_DECODER_H

#include "features/framework/core/tool/tool.h"

#include <QString>

/**
 * @brief JWT デコーダーのロジック
 * @details `header.payload.signature` を分割し、header / payload を Base64URL
 *          デコードして JSON 整形する。@n
 *          **署名の検証は行わない** (jwt.io と同様のデコード専用ツール)。@n
 *          処理は完全にローカルで完結し、ネットワーク送信は行わない。
 */
class JwtDecoder : public Tool
{
    Q_OBJECT

public:
    /**
     * @brief デコード結果の 1 パート (header / payload)
     */
    struct DecodedPart
    {
        /// Base64URL デコードと JSON パースの両方に成功したら true
        bool ok = false;
        /// 整形済み JSON (成功時のみ)
        QString formattedJson;
        /// 失敗理由 (翻訳済みメッセージ、失敗時のみ)
        QString error;
    };

    explicit JwtDecoder(QObject *parent = nullptr);
    JwtDecoder(const JwtDecoder &) = delete;
    JwtDecoder(JwtDecoder &&) = delete;
    JwtDecoder &operator=(const JwtDecoder &) = delete;
    JwtDecoder &operator=(JwtDecoder &&) = delete;
    ~JwtDecoder() override = default;

    /**
     * @brief デコード対象のトークンを設定する
     * @param token JWT 文字列
     */
    void setInputText(const QString &token);

    /**
     * @brief 現在の入力をデコードして結果を更新する
     * @details 例外は投げない。失敗は #header / #payload の DecodedPart::ok と
     *          #isValidStructure で表現する。
     */
    void decode();

    /// ヘッダーのデコード結果
    [[nodiscard]] const DecodedPart &header() const { return _header; }
    /// ペイロードのデコード結果
    [[nodiscard]] const DecodedPart &payload() const { return _payload; }
    /// 署名 (デコードせず Base64URL 文字列のまま)
    [[nodiscard]] const QString &signature() const { return _signature; }
    /// `header.payload[.signature]` の形になっていれば true
    [[nodiscard]] bool isValidStructure() const { return _validStructure; }

private:
    /**
     * @brief 1 セグメントを Base64URL デコードして JSON 整形する
     * @param segment Base64URL エンコードされたセグメント
     * @return デコード結果
     */
    static DecodedPart decodePart(const QString &segment);

    QString _inputText;
    DecodedPart _header;
    DecodedPart _payload;
    QString _signature;
    bool _validStructure = false;
};

#endif // JWT_DECODER_H
