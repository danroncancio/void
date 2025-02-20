#include "scene.hpp"

#include "engine.hpp"

namespace lum
{
    Scene::Scene() :
        assetMgr(Engine::Get().assetManager),
        renderer(Engine::Get().renderer)
    {};

    Scene::~Scene() = default;
}