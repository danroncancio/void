#ifndef SCENE_H
#define SCENE_H

#include "asset_manager.hpp"
#include "renderer.hpp"

namespace lum
{
    class Scene
    {
    public:
        bool loaded{};

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
    };
}

#endif // !SCENE_H
