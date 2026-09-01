#ifndef COMMAND_FUNCTION_H
#define COMMAND_FUNCTION_H
#pragma once
#include "command_option.h"

#include <QList>
#include <QString>

#include <utility>

/**
 * @brief コマンド機能を表すクラス
 * @details コマンドラインツールの機能情報とそのオプションリストを保持する
 */
class CommandFunction
{
public:
    /**
     * @brief コンストラクタ
     * @param name 機能名
     * @param description 機能の説明
     * @param options オプションのリスト
     */
    CommandFunction(QString name, QString description, const QList<CommandOption> &options)
        : name(std::move(name)), description(std::move(description)), options(options)
    {}

    /**
     * @brief 機能名を返す
     * @return 機能名
     */
    [[nodiscard]] QString getName() const { return name; }
    /**
     * @brief 機能の説明を返す
     * @return 機能の説明
     */
    [[nodiscard]] QString getDescription() const { return description; }
    /**
     * @brief オプションのリストを返す
     * @return オプションのリスト
     */
    [[nodiscard]] QList<CommandOption> getOptions() const { return options; }

private:
    /// 機能名
    QString name;
    /// 機能の説明
    QString description;
    /// オプションのリスト
    QList<CommandOption> options;
};
#endif // COMMAND_FUNCTION_H
