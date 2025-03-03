#ifndef SPRITE_COMP_H
#define SPRITE_COMP_H

#include "engine.hpp"
#include "components/drawable.hpp"
#include "components/translation.hpp"

namespace shmup
{
    class cSprite final : public cDrawable
    {
    public:
        cTranslation translation{};
        std::string  textureTag{};
        uint8_t      horizontalFrames{ 1 };
        uint8_t      verticalFrames{ 1 };
        uint8_t      currentFrame{};

    public:
        ~cSprite() = default;

        cSprite(const std::string &p_name) : cDrawable(p_name, DrawableType::SPRITE) {};

        void Update(float p_delta) override
        {
        }

        void Draw() override
        {
            lum::Engine::Get().renderer.AddToDrawQueue(this);
        }
    };
}

#endif // !SPRITE_COMP_H