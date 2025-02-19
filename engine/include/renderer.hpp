#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>

using namespace glm;

namespace lum
{
    struct WindowDesc
    {
        const char *title;
        vec2 size;
        vec2 resolution;
        vec2 upscaledResolution;
        vec2 targetOffset;
    };

    class Renderer
    {
    public:
        WindowDesc windowDesc;
        SDL_GPUDevice *gpuDevice;

    public:
        Renderer();
        ~Renderer();

        bool Init();
        void Shutdown();

        bool CreateWindowAndGPUDevice();

    private:
        SDL_Window *m_window;
    };
}

#endif // !RENDERER_H
