#ifndef LUM_COMPONENT_H
#define LUM_COMPONENT_H

#include <string>

namespace lum
{
    class Component
    {
    public:
        std::string name{};
        bool visual{};

    public:
        Component(const std::string &p_name, bool p_visual);
        virtual ~Component();

        virtual void Update(float) = 0;
        virtual void Draw();
    };
}

#endif // !LUM_COMPONENT_H