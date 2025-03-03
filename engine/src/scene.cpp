#include "scene.hpp"

#include "engine.hpp"

namespace lum
{
    Scene::Scene() :
        assetMgr(Engine::Get().assetManager),
        renderer(Engine::Get().renderer),
        audioMgr(Engine::Get().audioManager)
    {
    };

    Scene::~Scene() = default;

    void Scene::BindCommand(SDL_Scancode p_key, const char *p_command)
    {
        commandMap[p_key] = p_command;
    }

    void Scene::DoCommand(Command &p_command)
    {
        if (p_command.type == CommandType::START)
        {
            activeCommands.insert(p_command.name);
        }
        else if (p_command.type == CommandType::END)
        {
            activeCommands.erase(p_command.name);
        }
    }
}