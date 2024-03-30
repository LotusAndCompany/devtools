#ifndef TOOL_H
#define TOOL_H

#include <QObject>
#include "tool_id_fields.h"

// 各ツールの基底クラスにする
class Tool : public QObject
{
    Q_OBJECT

public:
    enum class ID {
        UNDEFINED,
        TOOL_ID_FIELDS, // ここに展開する
        // WIP
        MAX,
    };

    constexpr static const int ID_UNDEFINED = static_cast<int>(ID::UNDEFINED);
    constexpr static const int ID_MAX = static_cast<int>(ID::MAX);

    const ID id;

    virtual QString name() const = 0;
    virtual QString description() const = 0;

protected:
    explicit Tool(ID id);
    virtual ~Tool() = default;
};

#endif // TOOL_H
