#ifndef SYS_PLAYER_COMPONENT_H
#define SYS_PLAYER_COMPONENT_H

#include <glm/glm.hpp>

#include "scene.hpp"

using namespace glm;

namespace shmup
{
    class PlayerControllerSys
    {
    public:
        PlayerControllerSys(Scene *p_ctx) : ctx(p_ctx) {};

        void Update(Entity p_player, float p_delta)
        {
            auto &velo = ctx->ecs.GetComponent<cVelocity>(p_player)->get();

            velo.direction = vec2(0.0);

            if (ctx->activeCommands.count("MoveUp")) velo.direction.y += 1.0f;
            if (ctx->activeCommands.count("MoveDown")) velo.direction.y -= 1.0f;
            if (ctx->activeCommands.count("MoveRight")) velo.direction.x += 1.0f;
            if (ctx->activeCommands.count("MoveLeft")) velo.direction.x -= 1.0f;

            if (length(velo.direction) > 0)
                velo.direction = normalize(velo.direction);
        }

    private:
        Scene *ctx;
    };
}

#endif // !SYS_PLAYER_COMPONENT_H