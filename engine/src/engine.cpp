#include "engine.hpp"

#include <memory>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>

#include "../../game/scenes/test_ground_scn.hpp"

namespace lum
{
    std::unique_ptr<Engine> Engine::m_instance = nullptr;

    Engine::Engine() = default;

    Engine::~Engine() = default;

    Engine &Engine::Get()
    {
        if (!m_instance)
        {
            m_instance = std::make_unique<Engine>();
        }

        return *m_instance;
    }

    bool Engine::Init()
    {
        // Init engine modules

        if (!assetManager.Init())
        {
            SDL_Log("Failed to initialized asset manager");
            return false;
        }

        if (!renderer.Init())
        {
            SDL_Log("Failed to initilized renderer");
            return false;
        }

        if (!sceneManager.Init())
        {
            SDL_Log("Failed to initialized scene manager");
            return false;
        }

        // TEMPORAL!!!
        //
        // Been thinking of having the registry of autoloads and scenes via a 
        // 'config' file instead of doing it from code.

        sceneManager.RegisterScene("test_ground", std::make_shared<shmup::TestGroundScn>(), true);

        lastTime = SDL_GetTicksNS();

        SDL_Log("Engine initialized");

        return true;
    }

    void Engine::Shutdown()
    {
        SDL_Log("Engine shutdown called");

        sceneManager.Shutdown();
        assetManager.Shutdown();
        renderer.Shutdown();
    }

    void Engine::Input(SDL_Event *p_event)
    {
        ImGui_ImplSDL3_ProcessEvent(p_event);

        switch (p_event->type)
        {
        case SDL_EVENT_WINDOW_RESIZED:
            renderer.UpdateWindowSize(p_event->display.data1, p_event->display.data2);
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            minimized = true;
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            minimized = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (p_event->key.scancode == SDL_SCANCODE_F11)
            {
                renderer.ToggleWindowFullscreen();
            }
            break;
        default:
            break;
        }
    }

    void Engine::Update()
    {
        auto start = SDL_GetTicksNS();

        assetManager.CheckForModifiedAssets();

        currentTime = SDL_GetTicksNS();
        deltaTime = static_cast<float>(currentTime - lastTime) / SDL_NS_PER_SECOND;
        lastTime = currentTime;

        metricsWindows.StatsUpdate(deltaTime);

        sceneManager.currentScene->Update(deltaTime);

        auto end = SDL_GetTicksNS();
        metricsWindows.updateFrameTime = static_cast<float>(end - start) / SDL_NS_PER_MS;
    }

    bool Engine::Render()
    {
        auto start = SDL_GetTicksNS();

        renderer.PreRender();

        if (!minimized)
            sceneManager.currentScene->Draw();

        metricsWindows.ShowStatsWindows(deltaTime);

        if (!renderer.RenderFrame())
            return false;

        auto end = SDL_GetTicksNS();
        metricsWindows.renderFrameTime = static_cast<float>(end - start) / SDL_NS_PER_MS;

        return true;
    }
}