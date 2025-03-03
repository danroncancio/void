#include "actor.hpp"

namespace lum
{
    Actor::Actor() = default;

    Actor::~Actor() = default;

    void Actor::SetTag(const char *p_tag)
    {
        tag = p_tag;
    }

    void Actor::Update(float p_delta)
    {
        for (const auto &[_, comp] : m_components)
        {
            comp->Update(p_delta);
        }
    }

    void Actor::Draw()
    {
        for (const auto &[_, comp] : m_components)
        {
            if (comp->visual)
                comp->Draw();
        }
    }
}