#include "engine.hpp"

#include <memory>

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

        lastTime = SDL_GetTicks();

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
        currentTime = SDL_GetTicks();
        deltaTime = static_cast<float>(currentTime - lastTime) / SDL_MS_PER_SECOND;
        lastTime = currentTime;

        sceneManager.currentScene->Update(deltaTime);
    }

    bool Engine::Render()
    {
        renderer.PreRender();

        if (!minimized)
            sceneManager.currentScene->Draw();

        if (!renderer.RenderFrame())
            return false;

        return true;
    }
}