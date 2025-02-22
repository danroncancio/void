#ifndef SYS_DRAWING_H
#define SYS_DRAWING_H

#include "scene.hpp"

using namespace lum;

namespace shmup
{
    class DrawingSys
    {
    public:
        DrawingSys(Scene *p_ctx) : ctx(p_ctx)
        {
            drawableSig.set(ctx->ecs.GetComponentIndex<cSprite>());
            drawableSig.set(ctx->ecs.GetComponentIndex<cAnimSprite>());
            drawableSig.set(ctx->ecs.GetComponentIndex<cRectangle>());
        };

        void Draw()
        {
            auto drawableEntities = ctx->ecs.QueryEntitiesWithSignature(drawableSig, true);
            for (const auto &e : drawableEntities)
            {
                auto &tran = ctx->ecs.GetComponent<cTranslation>(e)->get();

                if (ctx->ecs.HasComponent<cSprite>(e))
                {
                    auto &sprt = ctx->ecs.GetComponent<cSprite>(e)->get();

                    ctx->renderer.AddToDrawQueue(DrawDesc{ tran, sprt });
                }
                else if (ctx->ecs.HasComponent<cAnimSprite>(e))
                {
                    auto &animSprt = ctx->ecs.GetComponent<cAnimSprite>(e)->get();

                    ctx->renderer.AddToDrawQueue(DrawDesc{ tran, animSprt.sprite });
                }
                
            }
        }

    private:
        Scene *ctx;
        Signature drawableSig;
    };
}

#endif // !SYS_DRAWING_H