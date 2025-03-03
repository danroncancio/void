#include "component.hpp"

namespace lum
{
    Component::Component(const std::string &p_name, bool p_visual) : name(p_name), visual(p_visual) {};

    Component::~Component() = default;

    void Component::Draw() {}
}