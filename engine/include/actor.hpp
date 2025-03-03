#ifndef LUM_ACTOR_H
#define LUM_ACTOR_H

#include <string>
#include <memory>
#include <unordered_map>

#include "component.hpp"

namespace lum
{
    class Actor
    {
    public:
        const char *tag{};

    public:
        Actor();
        virtual ~Actor();

        Actor(Actor &&other) noexcept : tag(other.tag), m_components(std::move(other.m_components))
        {
        }

        void SetTag(const char *p_tag);
        void Update(float p_delta);
        void Draw();

        template<typename T, typename... Args>
        T *AddComponent(const std::string &p_name, Args... p_args)
        {
            static_assert(std::is_base_of<lum::Component, T>::value, "T must derive from lum::Component");

            auto comp = std::make_unique<T>(p_name, std::forward<Args>(p_args)...);
            T *rawPtr = comp.get();

            m_components.emplace(p_name, std::move(comp));

            return rawPtr;
        }

        template<typename T>
        T *GetComponent(const std::string &p_name)
        {
            static_assert(std::is_base_of<lum::Component, T>::value, "T must derive from lum::Component");

            auto it = m_components.find(p_name);
            return (it != m_components.end()) ? static_cast<T *>(it->second.get()) : nullptr;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<Component>> m_components{};

    private:
        //Actor(const Actor &) = delete;
        //Actor &operator=(const Actor &) = delete;
    };
}

#endif // !LUM_ACTOR_H