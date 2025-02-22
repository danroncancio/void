#ifndef PLAYGROUND_LVL_H
#define PLAYGROUND_LVL_H

#include <memory>

using namespace lum;
using namespace glm;

#include "systems/entities_movement.hpp"
#include "systems/player_controller.hpp"
#include "systems/drawing.hpp"

namespace shmup
{
	class PlaygroundLvl final : public lum::Scene
	{
	public:
		Signature drawableSig;
		Entity ship;

		std::unique_ptr<EntitiesMovementSys> entitiesMovementSys{};
		std::unique_ptr<PlayerControllerSys> playerControllerSys{};
		std::unique_ptr<DrawingSys> drawingSys{};

	public:
		PlaygroundLvl() = default;
		~PlaygroundLvl() = default;

		void Setup() override
		{
			ecs.RegisterComponent<cTranslation>();
			ecs.RegisterComponent<cVelocity>();
			ecs.RegisterComponent<cSprite>();
			ecs.RegisterComponent<cAnimSprite>();
			ecs.RegisterComponent<cRectangle>();

			renderer.clearColor = vec4(0.1, 0.1, 0.1, 1.0);

			//
			// Load assets

			assetMgr.LoadTexture("ship", "sprites/player/ship.png");
			assetMgr.LoadTexture("ship_bullet", "sprites/player/ship_bullet.png");
			assetMgr.LoadTexture("ship_engine_fire", "sprites/player/ship_engine_fire.png");
			assetMgr.LoadTexture("ship_flash", "sprites/player/ship_weapon_flash.png");

			//
			// Bind commands to keys

			BindCommand(SDL_SCANCODE_UP, "MoveUp");
			BindCommand(SDL_SCANCODE_DOWN, "MoveDown");
			BindCommand(SDL_SCANCODE_RIGHT, "MoveRight");
			BindCommand(SDL_SCANCODE_LEFT, "MoveLeft");

			//
			// Create entities

			ship = ecs.CreateEntity();
			ecs.AddComponent(ship, cTranslation{ vec2(120.0f, 50.0f), 0.0f, 1.0f });
			ecs.AddComponent(ship, cVelocity{ vec2(0.0f), 150.0f });
			ecs.AddComponent(ship, cSprite{ "ship", 5, 1, 2 });

			auto ship_engine = ecs.CreateEntity();
			ecs.AddComponent(ship_engine, cTranslation{ vec2(120.0f, 150.0f), 0.0f, 1.0f });
			ecs.AddComponent(ship_engine, cAnimSprite{ cSprite{ "ship_engine_fire", 2 }, 15.0f});

			//
			// Systems

			drawingSys = std::make_unique<DrawingSys>(this);
			entitiesMovementSys = std::make_unique<EntitiesMovementSys>(this);
			playerControllerSys = std::make_unique<PlayerControllerSys>(this);
		}

		void Update(float p_delta) override
		{
			entitiesMovementSys->Update(p_delta);
			playerControllerSys->Update(ship, p_delta);

			// Animated sprite system

			ecs.ForEachEntityWithComponent<cAnimSprite>([&](cAnimSprite &p_animSprt)
				{
					p_animSprt.timer += p_delta;

					if (p_animSprt.timer >= (1 / p_animSprt.framerate))
					{
						p_animSprt.sprite.currentFrame = (p_animSprt.sprite.currentFrame + 1) % p_animSprt.sprite.horizontalFrames;

						p_animSprt.timer = 0.0f;
					}
				});
		}

		void Draw() override
		{
			drawingSys->Draw();
		}
	};
}

#endif // !PLAYGROUND_LVL_H