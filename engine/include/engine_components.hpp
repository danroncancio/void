#ifndef ENGINE_COMPONENTS_H
#define ENGINE_COMPONENTS_H

#include <variant>
#include <utility>

#include <glm/glm.hpp>

using namespace glm;

namespace lum
{
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

	struct cSprite
	{
		const char *tag;
		uint32_t horizontalFrames{ 1 };
		uint32_t verticalFrames{ 1 };
		uint32_t currentFrames{ 0 };
		DrawProperties properties{};
	};

	// Data types for renderer

	using DrawableVariant = std::variant<cSprite>;
	using DrawDesc = std::pair<cTranslation, DrawableVariant>;
}

#endif // !ENGINE_COMPONENTS_H