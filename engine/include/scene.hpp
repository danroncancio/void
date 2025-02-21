#ifndef SCENE_H
#define SCENE_H

#include "asset_manager.hpp"
#include "renderer.hpp"
#include "audio_manager.hpp"
#include "ecs.hpp"
#include "engine_components.hpp"

namespace lum
{
    class Scene
    {
    public:
        bool loaded{};
        ECS ecs;

    public:
        Scene();
        virtual ~Scene();

        virtual void Setup() = 0;
        virtual void Input() = 0;
        virtual void Update(float p_delta) = 0;
        virtual void Draw() = 0;

    protected:
        AssetManager &assetMgr;
        Renderer &renderer;
        AudioManager &audioMgr;
    };
}

#endif // !SCENE_H
