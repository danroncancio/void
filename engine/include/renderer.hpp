#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>

#include "renderer_types.hpp"

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
        vec4 clearColor{ 1.0f };

    public:
        Renderer();
        ~Renderer();

        bool Init();
        void Shutdown();

        void ToggleWindowFullscreen();
        void UpdateWindowSize(uint32_t p_width, uint32_t p_height);

        void PreRender();
        bool RenderFrame();

    private:
        mat4 m_projMat{};

        bool m_windowFullscreen{};
        SDL_Window *m_window;
        std::unordered_map<uint32_t, GraphicPipelineInfo> m_graphicsPipelines{};

        SDL_GPURenderPass *m_renderPass{};
        SDL_GPUCommandBuffer *m_commandBuffer{};

        SDL_GPUBuffer *m_rtVertexBuffer{};
        SDL_GPUBuffer *m_rtIndexBuffer{};
        SDL_GPUBuffer *m_quadVertexBuffer{};
        SDL_GPUBuffer *m_quadIndexBuffer{};

        SDL_GPUSampler *m_rtSampler{};

        SDL_GPUTexture *m_rtTexture{};

        SDL_GPUViewport m_windowViewport{};

        float time{};

    private:
        bool CreateWindowAndGPUDevice();
        bool CreateGraphicsPipeline(const char *p_tag, const char *p_vertTag, const char *p_fragTag, bool p_reload = false);
        SDL_GPUBuffer *CreateGPUBuffer(SDL_GPUBufferUsageFlags p_usage, uint32_t p_size, const char *p_debugName) const;
        bool SetupRenderTarget();
        bool SetupQuadData();
        bool SetupRenderTargetSampler();
        void CalculateRenderTargetResolution();
    };
}

#endif // !RENDERER_H
