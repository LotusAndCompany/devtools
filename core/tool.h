#ifndef TOOL_H
#define TOOL_H

// 各ツールの基底クラスにする
class Tool
{
public:
    Tool() = delete;

    enum class ID {
        UNDEFINED,
        API_TOOL,
        COMMAND_GENERATION,
        IMAGE_RESIZE,
        IMAGE_ROTATION,
        IMAGE_SPLIT,
        IMAGE_TRANSPARENT,
        IMAGE_TRIM,
        IMAGE_COMBINE,
        PHRASE_GENERATION,
        // WIP
        MAX,
    };

    const ID id;

protected:
    Tool(ID id);
    virtual ~Tool() = default;
};

#endif // TOOL_H
