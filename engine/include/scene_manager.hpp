#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <string>
#include <memory>
#include <unordered_map>

#include "scene.hpp"
#include "autoload.hpp"

namespace lum
{
    class SceneManager
    {
    public:
        std::shared_ptr<Scene> currentScene{};
        std::unordered_map<std::string, std::shared_ptr<Autoload>> autoloadRegister{};
        std::unordered_map<std::string, std::shared_ptr<Scene>> sceneRegister{};

    public:
        SceneManager();
        ~SceneManager();

        bool Init();
        void Shutdown();

        bool ChangeSceneTo(const std::string &p_tag);
        bool RegisterScene(const std::string &p_tag, std::shared_ptr<Scene> p_scene, bool p_setCurrent = false);
        bool LoadAutoload(const std::string &p_tag, std::shared_ptr<Autoload> p_autoload);

    private:
        SceneManager(const SceneManager &) = delete;
        SceneManager &operator=(const SceneManager &) = delete;

    };
}

#endif // !SCENE_MANAGER_H
