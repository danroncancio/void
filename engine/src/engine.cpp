#include "engine.hpp"

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

        return true;
    }

    void Engine::Shutdown()
    {
        sceneManager.Shutdown();
        assetManager.Shutdown();
        renderer.Shutdown();
    }

    void Engine::Input(SDL_Event *p_event)
    {
        switch (p_event->type)
        {
        case SDL_EVENT_WINDOW_RESIZED:
            renderer.UpdateWindowSize(p_event->display.data1, p_event->display.data2);
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
        
    }

    bool Engine::Render()
    {
        renderer.PreRender();
        if (!renderer.RenderFrame())
            return false;

        return true;
    }
}