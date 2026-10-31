#include "tool.h"

#include "features/framework/core/exception/invalid_argument_exception.h"

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
    : QObject(parent), stringID(std::move(stringID))
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
