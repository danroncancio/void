#ifndef TRANSLATION_COMP_H
#define TRANSLATION_COMP_H

#include <glm/glm.hpp>

using namespace glm;

namespace shmup
{
    class cTranslation
    {
    public:
        vec2  position{};
        float rotation{};
        float scale{ 1.0f };

    public:
        cTranslation() = default;
        ~cTranslation() = default;
    };
}

#endif // !TRANSLATION_COMP_H