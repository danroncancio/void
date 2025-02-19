#include "renderer.hpp"

#include <SDL3_shadercross/SDL_shadercross.h>

namespace lum
{
    Renderer::Renderer() = default;

    Renderer::~Renderer() = default;

    bool Renderer::Init()
    {
        windowDesc.title = "void";
        windowDesc.size = vec2(1280, 720);
        windowDesc.resolution = vec2(240, 360);

        if (!CreateWindowAndGPUDevice())
            return false;

        // Load shaders

        // Create graphics pipeline

        // Create texture sampler

        // Setup Main render target texture

        // Setup texture quad data


        return true;
    }

    void Renderer::Shutdown()
    {
        SDL_DestroyGPUDevice(gpuDevice);

        SDL_DestroyWindow(m_window);
    }

    bool Renderer::CreateWindowAndGPUDevice()
    {
        m_window = SDL_CreateWindow(windowDesc.title, static_cast<int>(windowDesc.size.x), static_cast<int>(windowDesc.size.y), 0);
        if (!m_window)
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create SDL window: %s", SDL_GetError());
            return false;
        }

        gpuDevice = SDL_CreateGPUDevice(SDL_ShaderCross_GetSPIRVShaderFormats(), true, nullptr);
        if (!gpuDevice)
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create SDL GPU device: %s", SDL_GetError());
            return false;
        }

        if (!SDL_ClaimWindowForGPUDevice(gpuDevice, m_window))
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to claim window for GPU device: %s", SDL_GetError());
            return false;
        }

        SDL_SetGPUSwapchainParameters(gpuDevice, m_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

        return true;
    }
}