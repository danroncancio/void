#ifndef TEST_GROUND_SCN_H
#define TEST_GROUND_SCN_H

#include "scene.hpp"

using namespace glm;
using namespace lum;

namespace shmup
{
	enum class Cmds
	{
		ONE,
		TWO,
	};

	class TestGroundScn final : public lum::Scene
	{
	public:
		Entity skull{};
		Signature drawablesSig{};
		Signature movablesSig{};

	public:
		TestGroundScn() = default;

		void Setup() override
		{
			SDL_srand(1337);

			renderer.clearColor = vec4(0.8, 0.3, 0.4, 1.0);

			assetMgr.LoadTexture("skull", "sprites/skull.png");
			assetMgr.LoadSound("wind", "sounds/music/windchill.ogg");

			BindCommand(SDL_SCANCODE_UP, "MoveUp");
			BindCommand(SDL_SCANCODE_DOWN, "MoveDown");
			BindCommand(SDL_SCANCODE_RIGHT, "MoveRight");
			BindCommand(SDL_SCANCODE_LEFT, "MoveLeft");

			ecs.RegisterComponent<cTranslation>();
			ecs.RegisterComponent<cVelocity>();
			ecs.RegisterComponent<cSprite>();


			auto skull = ecs.CreateEntity();
				
			ecs.AddComponent(skull, cTranslation{ vec2(120.0f, 50.0f), 0.0f, 1.0f});
			ecs.AddComponent(skull, cVelocity{ vec2(0.0), 150.0f });
			ecs.AddComponent(skull, cSprite{ "skull" });


			drawablesSig.set(ecs.GetComponentIndex<cTranslation>());
			drawablesSig.set(ecs.GetComponentIndex<cSprite>());

			movablesSig.set(ecs.GetComponentIndex<cTranslation>());
			movablesSig.set(ecs.GetComponentIndex<cVelocity>());
		};

		void Update(float p_delta) override
		{
			// Player movement system

			auto &velo = ecs.GetComponent<cVelocity>(skull)->get();

			velo.direction = vec2(0.0);

			if (activeCommands.count("MoveUp")) velo.direction.y += 1.0f;
			if (activeCommands.count("MoveDown")) velo.direction.y -= 1.0f;
			if (activeCommands.count("MoveRight")) velo.direction.x += 1.0f;
			if (activeCommands.count("MoveLeft")) velo.direction.x -= 1.0f;

			if (length(velo.direction) > 0)
				velo.direction = normalize(velo.direction);

			
			// Move system

			auto movableEntities = ecs.QueryEntitiesWithSignature(movablesSig);
			for (const auto &e : movableEntities)
			{
				auto &tran = ecs.GetComponent<cTranslation>(e)->get();
				auto &velo = ecs.GetComponent<cVelocity>(e)->get();

				tran.position += velo.direction * velo.speed * p_delta;
			}
		};

		void Draw() override
		{
			auto drawableEntities = ecs.QueryEntitiesWithSignature(drawablesSig);
			for (const auto &e : drawableEntities)
			{
				auto &tran = ecs.GetComponent<cTranslation>(e)->get();
				auto &sprt = ecs.GetComponent<cSprite>(e)->get();

				renderer.AddToDrawQueue(DrawDesc{ tran, sprt });
			}
		};
	};
}

#endif // !TEST_GROUND_SCN_H