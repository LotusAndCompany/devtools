#ifndef TOOL_H
#define TOOL_H

#include <QObject>
#include "tool_id_fields.h"

// 各ツールの基底クラスにする
class Tool : public QObject
{
    Q_OBJECT

public:
    Tool() = delete;
    Tool(const Tool &) = delete;

    enum class ID {
        UNDEFINED,
        TOOL_ID_FIELDS, // ここに展開する
        // WIP
        MAX,
    };
    const ID id;
    const QString stringID;

    // 翻訳が必要な情報
    struct Info
    {
        QString name;
        QString description;
    };

    constexpr static const int ID_UNDEFINED = static_cast<int>(ID::UNDEFINED);
    constexpr static const int ID_MAX = static_cast<int>(ID::MAX);

    static const Info info(ID id) noexcept(false);
    const QString &toolName() const { return _info.name; }
    const QString &toolDesctiption() const { return _info.description; }

protected:
    explicit Tool(ID id, const QString &stringID, QObject *parent = nullptr);
    virtual ~Tool() = default;

    static const QString invalidToolIDReason;

    static void validateID(ID id) noexcept(false);

    virtual bool event(QEvent *event) override;

private:
    Info _info;
};

#endif // TOOL_H
