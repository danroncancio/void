#ifndef ASSET_TYPES_H
#define ASSET_TYPES_H

#include <SDL3/SDL.h>

namespace lum
{
    enum class ShaderType
    {
        VERTEX,
        FRAGMENT
    };

    struct Shader
    {
        const char     *tag;
        ShaderType      type{};
        SDL_GPUShader  *data{};
        const char     *filePath{};
        SDL_Time        lastModifyTime{};
    };
}

#endif // !ASSET_TYPES_H
