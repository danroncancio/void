#ifndef PLAYGROUND_LVL_H
#define PLAYGROUND_LVL_H

#include <vector>

#include "actors/player.hpp"
#include "components/sprite.hpp"
#include "components/anim_sprite.hpp"

using namespace lum;
using namespace glm;

namespace shmup
{
	class PlaygroundLvl final : public lum::Scene
	{
	public:
		aPlayer ship{};

	public:
		PlaygroundLvl() = default;
		~PlaygroundLvl() = default;

		void Setup() override
		{
			renderer.clearColor = vec4( 0.1f, 0.1f, 0.1f, 1.0f );

			assetMgr.LoadTexture("ship_body", "sprites/player/ship.png");
			assetMgr.LoadTexture("ship_engine_fire", "sprites/player/ship_engine_fire.png");

			auto bodySpriteComp = ship.AddComponent<cSprite>("body_sprite");
			bodySpriteComp->translation.position = vec2(140.0f, 90.0f);
			bodySpriteComp->textureTag = "ship_body";
			bodySpriteComp->horizontalFrames = 5;
			bodySpriteComp->currentFrame = 2;

			auto engineFireComp = ship.AddComponent<cAnimSprite>("ship_engine_fire");
			engineFireComp->translation.position = vec2(100.0f, 50.0f);
			engineFireComp->textureTag = "ship_engine_fire";
			engineFireComp->horizontalFrames = 2;
			engineFireComp->framerate = 15;
		}

		void Update(float p_delta) override
		{
			ship.Update(p_delta);
		}

		void Draw() override
		{
			ship.Draw();
		}
	};
}

#endif // !PLAYGROUND_LVL_H