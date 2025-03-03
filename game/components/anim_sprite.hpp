#ifndef ANIM_SPRITE_COMP_H
#define ANIM_SPRITE_COMP_H

#include "component.hpp"

namespace shmup
{
    class cAnimSprite final : public cDrawable
    {
    public:
        cTranslation translation{};
        std::string  textureTag{};
        uint8_t      horizontalFrames{ 1 };
        uint8_t      verticalFrames{ 1 };
        uint8_t      currentFrame{};
        float        framerate{};
        float        timer{};
        bool         loop{ true };

    public:
        cAnimSprite(const std::string &p_name) : cDrawable(p_name, DrawableType::ANIM_SPRITE) {}
        ~cAnimSprite() = default;

        void Update(float p_delta) override
        {
            timer += p_delta;

            if (timer >= (1 / framerate))
            {
                currentFrame = (currentFrame + 1) % horizontalFrames;
                timer = 0.0f;
            }            
        }

        void Draw() override
        {
            lum::Engine::Get().renderer.AddToDrawQueue(this);
        }
    };
}

#endif // !ANIM_SPRITE_COMP_H