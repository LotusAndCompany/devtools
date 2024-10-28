#ifndef BASE_EMITTER_H
#define BASE_EMITTER_H

#include <QString>

class BaseEmitter
{
public:
    virtual ~BaseEmitter();

    /// 出力形式を定義した列挙体
    enum class Style {
        MIN,             ///<最小値
        INDENT_2_SPACES, ///<2スペースでインデント
        INDENT_4_SPACES, ///<4スペースでインデント
        INDNENT_TABS,    ///<tabでインデント
        OPTIMIZED,       ///<可能な限り小さくする
        MAX,             ///<最大値
    };

    virtual QString emitSerialized(const QVariant &input, Style style) = 0;

#ifdef _TEST_BasicEmitter
    friend class Test::TestBasicParser;
#endif
};

#endif // BASE_EMITTER_H
