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
        MIN,
        TOOL_ID_FIELDS(), // ここに展開する
        MAX,
    };
    const QString stringID;

    // 翻訳が必要な情報
    struct Translatable
    {
        QString name;
        // TODO: ツールの簡単な説明として表示できるようにする
        QString description;
    };

    constexpr static const int ID_MIN = static_cast<int>(ID::MIN);
    constexpr static const int ID_MAX = static_cast<int>(ID::MAX);

    static const Translatable translatable(ID id) noexcept(false);
    const Translatable &translatable() const noexcept(false) { return _translatable; }

protected:
    // NOTE: idによっては例外が発生する
    explicit Tool(ID id, const QString &stringID, QObject *parent = nullptr) noexcept(false);
    virtual ~Tool() = default;

    static const QString invalidToolIDReason;

    static void validateID(ID id) noexcept(false);

    virtual bool event(QEvent *event) override;

private:
    const ID id;
    Translatable _translatable;
};

#endif // TOOL_H
