#ifndef COMMAND_OPTION_H
#define COMMAND_OPTION_H
#pragma once
#include <QString>

#include <utility>

/**
 * @brief コマンドオプションを表すクラス
 * @details コマンドラインツールのオプション情報を保持する
 */
class CommandOption
{
public:
    /**
     * @brief コンストラクタ
     * @param name オプション名
     * @param required 必須オプションの場合は `true`
     * @param title オプションのタイトル
     * @param requiredQuotes 引用符が必要な場合は `true`
     */
    CommandOption(QString name, bool required, QString title, bool requiredQuotes)
        : name(std::move(name))
        , required(required)
        , title(std::move(title))
        , requiredQuotes(requiredQuotes)
    {}

    /**
     * @brief オプション名を返す
     * @return オプション名
     */
    [[nodiscard]] QString getName() const { return name; }
    /**
     * @brief 必須オプションかどうかを返す
     * @return 必須オプションの場合は `true`
     */
    [[nodiscard]] bool isRequired() const { return required; }
    /**
     * @brief オプションのタイトルを返す
     * @return オプションのタイトル
     */
    [[nodiscard]] QString getTitle() const { return title; }
    /**
     * @brief 引用符が必要かどうかを返す
     * @return 引用符が必要な場合は `true`
     */
    [[nodiscard]] bool isRequiredQuotes() const { return requiredQuotes; }

private:
    /// オプション名
    QString name;
    /// 必須オプションかどうか
    bool required;
    /// オプションのタイトル
    QString title;
    /// 引用符が必要かどうか
    bool requiredQuotes;
};

#endif // COMMAND_OPTION_H
