#ifndef ASSET_TYPES_H
#define ASSET_TYPES_H

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

using namespace glm;

namespace lum
{
    enum class ShaderType
    {
        VERTEX,
        FRAGMENT
    };

    struct Shader
    {
        const char     *tag{};
        ShaderType      type{};
        SDL_GPUShader  *data{};
        const char     *filePath{};
        SDL_Time        lastModifyTime{};
    };

    struct Texture
    {
        const char     *tag{};
        vec2            size{};
        SDL_GPUTexture *data{};
        const char     *filePath{};
        SDL_Time        lastModifyTime{};
    };

    struct Sound
    {
        const char          *tag{};
        SDL_AudioSpec        audioSpec{};
        std::vector<uint8_t> buffer;
        uint32_t             length{};
        std::string          filePath{};
        SDL_Time             lastModifyTime{};
    };
}

#endif // !ASSET_TYPES_H
