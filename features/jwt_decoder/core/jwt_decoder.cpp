#include "jwt_decoder.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QStringList>

namespace {
/// JWS のセグメント数 (signature を省いた 2 パートも許容する)
constexpr int MIN_SEGMENT_COUNT = 2;
constexpr int MAX_SEGMENT_COUNT = 3;
} // namespace

JwtDecoder::JwtDecoder(QObject *parent) : Tool(Tool::ID::JWT_DECODER, "jwt-decoder", parent) {}

void JwtDecoder::setInputText(const QString &token)
{
    _inputText = token;
}

void JwtDecoder::decode()
{
    _header = DecodedPart{};
    _payload = DecodedPart{};
    _signature.clear();
    _validStructure = false;

    // NOTE: 貼り付け時の前後の空白・改行は無視する
    const QString token = _inputText.trimmed();
    if (token.isEmpty()) {
        return;
    }

    const QStringList segments = token.split('.');
    if (segments.size() < MIN_SEGMENT_COUNT || MAX_SEGMENT_COUNT < segments.size()) {
        return;
    }
    if (segments.at(0).isEmpty() || segments.at(1).isEmpty()) {
        return;
    }

    _validStructure = true;

    // 各パートは独立して処理し、片方が失敗しても他方は表示できるようにする
    _header = decodePart(segments.at(0));
    _payload = decodePart(segments.at(1));
    _signature = segments.value(MAX_SEGMENT_COUNT - 1);
}

JwtDecoder::DecodedPart JwtDecoder::decodePart(const QString &segment)
{
    DecodedPart part;

    // JWT はパディングを省略するため Base64UrlEncoding でデコードする。
    // 不正な文字を検出するために fromBase64Encoding の戻り値でステータスを確認する。
    const QByteArray::FromBase64Result result = QByteArray::fromBase64Encoding(
        segment.toUtf8(), QByteArray::Base64UrlEncoding | QByteArray::AbortOnBase64DecodingErrors);
    if (result.decodingStatus != QByteArray::Base64DecodingStatus::Ok) {
        part.error = tr("Failed to Base64URL-decode");
        return part;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(*result, &parseError);
    if (parseError.error != QJsonParseError::NoError || document.isNull()) {
        part.error = tr("Invalid JSON");
        return part;
    }

    part.ok = true;
    part.formattedJson = QString::fromUtf8(document.toJson(QJsonDocument::Indented)).trimmed();

    return part;
}
