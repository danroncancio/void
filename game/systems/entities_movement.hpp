#ifndef SYS_ENTITIES_MOVEMENT_H
#define SYS_ENTITIES_MOVEMENT_H

#include "scene.hpp"

using namespace lum;
using namespace glm;

namespace shmup
{
    class EntitiesMovementSys
    {
    public:
        EntitiesMovementSys(Scene *p_ctx) : ctx(p_ctx)
        {
            movableEntitiesSig.set(ctx->ecs.GetComponentIndex<cTranslation>());
            movableEntitiesSig.set(ctx->ecs.GetComponentIndex<cVelocity>());
        }

        void Update(float p_delta)
        {
            auto movableEntities = ctx->ecs.QueryEntitiesWithSignature(movableEntitiesSig);
            for (const auto &e : movableEntities)
            {
                auto &trans = ctx->ecs.GetComponent<cTranslation>(e)->get();
                auto &velo = ctx->ecs.GetComponent<cVelocity>(e)->get();

                trans.position += velo.direction * velo.speed * p_delta;
            }
        }

    private:
        Scene *ctx;
        Signature movableEntitiesSig;
    };
}

#endif // !SYS_ENTITIES_MOVEMENT_H