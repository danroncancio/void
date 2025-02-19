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
        renderer.Shutdown();
    }

    void Engine::Input(SDL_Event *p_event)
    {
        switch (p_event->type)
        {
        default:
            break;
        }
    }

    void Engine::Update()
    {

    }

    void Engine::Render()
    {

    }
}