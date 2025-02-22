#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

using namespace glm;

namespace lum
{
    struct PosVertex
    {
        vec3 position;
    };

    struct PosTexVertex
    {
        vec3 position;
        vec2 texCoord;
    };

    struct ProjMatUniform
    {
        mat4 model;
        mat4 view;
        mat4 proj;
    };

    struct TimeColorUniform
    {
        float time;
        int horizontalFrames;
        int currentFrame;
        alignas(16) vec4 modulateColor;
    };

    struct GraphicPipelineInfo
    {
        const char *tag;
        SDL_GPUGraphicsPipeline *pipeline;
        const char *vertTag;
        const char *fragTag;
    };
}

#endif // !RENDERER_TYPES_H
