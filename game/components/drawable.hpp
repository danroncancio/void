#ifndef DRAWABLE_COMP_H
#define DRAWABLE_COMP_H

#include <string>

#include <glm/glm.hpp>

#include "component.hpp"

namespace shmup
{
    enum class DrawableType
    {
        SPRITE,
        ANIM_SPRITE,
        SHAPE_RECT,
        SHAPE_CIRC,
    };

    class cDrawable : public lum::Component
    {
    public:
        DrawableType drawableType{};
        bool         visible{ true };
        uint8_t      layer{};
        vec4         modulateColor{ 1.0f };

    public:
        ~cDrawable() = default;

        cDrawable(const std::string &p_name, const DrawableType p_drawableType)
            : lum::Component(p_name, true), drawableType(p_drawableType) 
        {
        };

        void Update(float) override {}
    };
}

#endif // !DRAWABLE_COMP_H