#ifndef ENGINE_COMPONENTS_H
#define ENGINE_COMPONENTS_H

#include <variant>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

#include "ecs.hpp"

using namespace glm;

namespace lum
{
	struct cParent
	{
		Entity parent;
	};

	struct cChildren
	{
		std::vector<Entity> children;
	};

	struct cTranslation
	{
		vec2 position{};
		float rotation{};
		float scale{};
	};

	struct cVelocity
	{
		vec2 direction{};
		float speed{};
	};

	struct DrawProperties
	{
		bool visible{ true };
		uint32_t layer{ 0 };
		vec4 modulateColor{ 1.0f };
	};

	struct cRectangle
	{
		vec2 size{};
		DrawProperties properties{};
	};

	struct cSprite
	{
		const char *tag;
		uint32_t horizontalFrames{ 1 };
		uint32_t verticalFrames{ 1 };
		uint32_t currentFrame{ 0 };
		DrawProperties properties{};
	};

	struct cAnimSprite
	{
		cSprite sprite{};
		float framerate{};
		float timer{};
		DrawProperties properties{};
	};

	// Data types for renderer

	using DrawableVariant = std::variant<cRectangle, cSprite, cAnimSprite>;
	using DrawDesc = std::pair<cTranslation, DrawableVariant>;
}

#endif // !ENGINE_COMPONENTS_H