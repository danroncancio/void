#include "scene_manager.hpp"

namespace lum
{
    SceneManager::SceneManager() = default;

    SceneManager::~SceneManager() = default;

    bool SceneManager::Init()
    {
        return true;
    }

    void SceneManager::Shutdown()
    {
    }

    bool SceneManager::ChangeSceneTo(const std::string &p_tag)
    {
        auto it = sceneRegister.find(p_tag);
        if (it == sceneRegister.end())
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SceneMgr: Failed to find scene: %s", p_tag.c_str());
            return false;
        }

        currentScene.reset();
        currentScene = it->second;

        // Setup scene if its being loaded for the first time or if it was unloaded previously
        if (!currentScene->loaded)
        {
            currentScene->Setup();
            currentScene->loaded = true;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SceneMgr: Scene had setup method called: %s", p_tag.c_str());
        }

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SceneMgr: Current scene changed to: %s", p_tag.c_str());

        return true;
    }

    bool SceneManager::RegisterScene(const std::string &p_tag, std::shared_ptr<Scene> p_scene, bool p_setCurrent)
    {
        sceneRegister.emplace(p_tag, std::move(p_scene));
        if (sceneRegister.at(p_tag))
        {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SceneMgr: Registered scene with tag: %s", p_tag.c_str());
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SceneMgr: Failed to register scene: %s", p_tag.c_str());
            return false;
        }

        if (p_setCurrent)
        {
            if (!ChangeSceneTo(p_tag))
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SceneMgr: Failed to change scene to: %s", p_tag.c_str());
                return false;
            }
        }

        return true;
    }

    bool SceneManager::LoadAutoload(const std::string &p_tag, std::shared_ptr<Autoload> p_autoload)
    {
        autoloadRegister.emplace(p_tag, std::move(p_autoload));

        auto &al = autoloadRegister.at(p_tag);
        if (al)
        {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SceneMgr: Registered autoload with tag: %s", p_tag.c_str());
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SceneMgr: Failed to register autoload: %s", p_tag.c_str());
            return false;
        }

        // Load autoload
        if (!al->loaded)
        {
            al->Setup();
            al->loaded = true;
        }

        return true;
    }
}