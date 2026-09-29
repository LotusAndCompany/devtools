#include "tool.h"

#include "features/framework/core/exception/invalid_argument_exception.h"
#include "features/framework/core/exception/under_development_exception.h"

#include <QEvent>

#include <utility>

/**
 * @brief ツール ID の有効範囲を示す、初回呼び出し時に構築した例外メッセージを返す
 */
const QString &Tool::invalidToolIDReason()
{
    static const QString reason =
        QString("Tool::ID must be in range (%1, %2)").arg(Tool::ID_MIN).arg(Tool::ID_MAX);
    return reason;
}

Tool::Tool(Tool::ID id, QString stringID, QObject *parent)
    : QObject(parent), id(id), stringID(std::move(stringID)), _translatable(translatable(id))
{
    validateID(id);
}

/**
 * @brief ツール ID が境界値を除く有効範囲内であることを確認する
 * @param id 検証するツール ID
 * @throws InvalidArgumentException<int> ID が範囲外の場合
 */
void Tool::validateID(ID id)
{
    const int intID = static_cast<int>(id);

    if (intID <= ID_MIN || ID_MAX <= intID) {
        throw InvalidArgumentException(intID, invalidToolIDReason());
    }
}

/**
 * @brief 指定ツールの名前と説明を現在の言語で返す
 * @param id 情報を取得するツール ID
 * @return 翻訳済みの名前と説明
 * @throws InvalidArgumentException<int> ID が範囲外の場合
 * @throws UnderDevelopmentException ID に対応する翻訳情報が未実装の場合
 */
Tool::Translatable Tool::translatable(ID id)
{
    validateID(id);

    // TODO: return Translatable{tr("Tool Name"), tr("Tool description")}
    switch (id) {
    case ID::IMAGE_ALL_IN_ONE:
        return Translatable{
            tr("Image Editor"),
            tr("Use resize, rotation, division, and transparent processing on one image"),
        };
    case ID::PHRASE_GENERATION:
        return Translatable{
            tr("Phrase Generation"),
            tr("Generate and manage Phrase"),
        };
    case ID::COMMAND_GENERATION:
        return Translatable{
            tr("Command Generation"),
            tr("Generate command from command list"),
        };
    case ID::DATA_CONVERSION:
        return Translatable{
            tr("Data/Format Conversion"),
            tr("Conversion and formatting JSON/YAML/TOML data"),
        };
    case ID::HTTP_REQUEST:
        return Translatable{tr("HTTP Request"), tr("Send HTTP Request")};
    case ID::QR_CODE_GENERATION:
        return Translatable{tr("QR Code Generation"), tr("Generate QR codes from text or URLs")};
    case ID::DB_TOOL:
        return Translatable{tr("DB Tool"), tr("Provides database-related functionalities")};
    case ID::MARKDOWN_PREVIEW:
        return Translatable{tr("Markdown Preview"), tr("Live preview of Markdown source")};
    case ID::REGEX_TESTER:
        return Translatable{tr("Regex Tester"), tr("Test and debug regular expressions")};
    case ID::DIFF_TOOL:
        return Translatable{
            tr("Diff Comparison Tool"),
            tr("Compare two texts and show the differences"),
        };
    default:
        throw UnderDevelopmentException();
    }
}

/**
 * @brief 言語変更時にツールの翻訳情報を更新する
 * @param event 処理するイベント
 * @return 言語変更なら true、それ以外は QObject の処理結果
 */
bool Tool::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        QObject::event(event);
        _translatable = translatable(id);
        return true;
    }
    return QObject::event(event);
}
