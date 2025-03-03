#ifndef COMMAND_H
#define COMMAND_H

#include <SDL3/SDL.h>

namespace lum
{
    enum class CommandType
    {
        START,
        END,
    };

    struct Command
    {
        const char *name{};
        CommandType type{};
    };
}

#endif // !COMMAND_H
