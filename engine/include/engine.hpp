#ifndef ENGINE_H
#define ENGINE_H

#include <memory>

#include <SDL3/SDL.h>

#include "scene_manager.hpp"

namespace lum
{
    class Engine
    {
    public:
        SceneManager sceneManager;

        float deltaTime{};
        float scaledDeltaTime{};
        float engineTime{};
        float timeScalar{1.0f};

    public:
        Engine();
        ~Engine();

        static Engine &Get();
        bool Init();
        void Shutdown();

        void Input(SDL_Event *p_event);
        void Update();
        void Render();

    private:
        static std::unique_ptr<Engine> m_instance;

    private:
        Engine(const Engine &) = delete;
        Engine &operator=(const Engine &) = delete;
    };
}

#endif // !ENGINE_H