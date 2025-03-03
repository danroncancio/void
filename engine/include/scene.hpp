#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "asset_manager.hpp"
#include "renderer.hpp"
#include "audio_manager.hpp"
#include "command.hpp"

namespace lum
{
    class Scene
    {
    public:
        bool loaded{};
        AssetManager &assetMgr;
        Renderer &renderer;
        AudioManager &audioMgr;

        std::unordered_map<SDL_Scancode, const char *> commandMap;
        std::unordered_set<const char *> activeCommands;

    public:
        Scene();
        virtual ~Scene();

        virtual void Setup() = 0;
        virtual void Update(float p_delta) = 0;
        virtual void Draw() = 0;

        void BindCommand(SDL_Scancode p_key, const char *p_command);
        void DoCommand(Command &p_command);
    };
}

#endif // !SCENE_H
