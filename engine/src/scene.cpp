#include "scene.hpp"

#include "engine.hpp"

namespace lum
{
    Scene::Scene() :
        assetMgr(Engine::Get().assetManager),
        renderer(Engine::Get().renderer),
        audioMgr(Engine::Get().audioManager)
    {};

    Scene::~Scene() = default;
}