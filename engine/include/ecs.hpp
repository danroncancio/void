#ifndef ECS_H
#define ECS_H

#include "SDL3/SDL_assert.h"
#include "SDL3/SDL_log.h"
#include <array>
#include <bitset>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>

namespace lum
{
    constexpr size_t MAX_ENTITIES = 1024;
    constexpr size_t MAX_COMPONENTS = 32;

    using Entity = size_t;
    using Signature = std::bitset<MAX_COMPONENTS>;

    class ECS
    {
    public:
        size_t nextEntityID{};
        size_t aliveEntitiesCount{};

    public:
        ECS() = default;

        Entity CreateEntity()
        {
            Entity ent;

            // Check if there's dead entities to reuse, if none then assign a new ID.

            if (!m_deadEntities.empty())
            {
                ent = m_deadEntities.front();
                m_deadEntities.pop();
            }
            else
            {
                ent = nextEntityID++;
            }

            aliveEntitiesCount++;

            // Assign Signature and setting its first bit (0 Alive)

            m_entitySignatures[ent].set(0);

            return ent;
        }

        void DeleteEntity(Entity p_entity)
        {
            // Check if the entity is already dead

            if (!m_entitySignatures[p_entity][0])
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Entity %zu already dead :(", p_entity);
                return;
            }

            // Reset its signature

            m_entitySignatures[p_entity].reset();

            // Queue it for later reuse

            m_deadEntities.push(p_entity);

            aliveEntitiesCount--;
        }

        template <typename T>
        void RegisterComponent()
        {
            auto typeIdx = std::type_index(typeid(T));

            // Check if the component was already registered

            if (auto it = m_componentDirectory.find(typeIdx); it != m_componentDirectory.end())
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ECS: Component '%s' was already registered", typeIdx.name());
                return;
            }

            SDL_assert(m_nextComponentIndex < MAX_COMPONENTS);

            // Assign to the new component an index

            m_componentIndex[typeIdx] = m_nextComponentIndex++;

            // Create a unique pointer to a component "table" already with space for the max entities

            auto compTable = std::make_shared<std::array<T, MAX_ENTITIES>>();

            // Add the component "table" to the directory

            m_componentDirectory[typeIdx] = std::move(compTable);

            SDL_Log("ECS: Component '%s' registered succesfully", typeIdx.name());
        }

        template <typename T>
        void AddComponent(Entity p_entity, T p_component)
        {
            // Check if the entity is alive

            if (IsDead(p_entity))
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ECS: Couldn't ADD component. The entity '%zu' is fucking dead!",
                    p_entity);
                return;
            }

            auto typeIdx = std::type_index(typeid(T));

            // Check if the component is already registered

            auto it = m_componentDirectory.find(typeIdx);
            if (it == m_componentDirectory.end())
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "ECS: Component '%s' not found in the directory when trying to add it to entity '%zu'",
                    typeIdx.name(), p_entity);
                return;
            }

            // Get the component table by casting it and replacing/assigning the component to the entity

            auto compTable = std::static_pointer_cast<std::array<T, MAX_ENTITIES>>(it->second);
            (*compTable)[p_entity] = p_component;

            // Get the component index and use it to set the entity signature

            size_t compIdx = m_componentIndex[typeIdx];
            m_entitySignatures[p_entity].set(compIdx);
        }

        template <typename T>
        void RemoveComponent(Entity p_entity)
        {
            auto typeIdx = std::type_index(typeid(T));

            // Check if the component is registered

            auto it = m_componentDirectory.find(typeIdx);
            if (it == m_componentDirectory.end())
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "ECS: Component '%s' not found when trying to remove it from entity '%zu'", typeIdx.name(),
                    p_entity);
                return;
            }

            // Check if the entity has the component

            if (!m_entitySignatures[p_entity][GetComponentIndex<T>()])
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ECS: Entity '%zu' does not have component '%s' to remove",
                    p_entity, typeIdx.name());
                return;
            }

            // Get the component table by casting it

            auto &comp = m_componentDirectory.at(typeIdx);
            auto  compTable = std::static_pointer_cast<std::array<T, MAX_ENTITIES>>(comp);

            // Reset the component for the entity

            (*compTable)[p_entity] = T(); // Reset to default value of T

            // Update entity signature to remove the component

            size_t compIdx = GetComponentIndex<T>();
            m_entitySignatures[p_entity].reset(compIdx);
        }

        template <typename T>
        std::optional<std::reference_wrapper<T>> GetComponent(Entity p_entity)
        {
            // Check if the entity is alive

            if (IsDead(p_entity))
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ECS: Couldn't GET component. The entity '%zu' is fucking dead!",
                    p_entity);
                return std::nullopt;
            }

            auto typeIdx = std::type_index(typeid(T));

            // Check if the component is registered

            if (auto it = m_componentDirectory.find(typeIdx); it == m_componentDirectory.end())
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "ECS: Component '%s' has not been registered", typeIdx.name());
                return std::nullopt;
            }

            // Check if the entity has the component

            if (!m_entitySignatures[p_entity][GetComponentIndex<T>()])
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ECS: Couldn't found the component '%s' for the entity '%zu'",
                    typeIdx.name(), p_entity);
                return std::nullopt;
            }

            // Get the component table by casting it

            auto &comp = m_componentDirectory.at(typeIdx);
            auto  compTable = std::static_pointer_cast<std::array<T, MAX_ENTITIES>>(comp);

            return (*compTable)[p_entity];
        }

        template <typename T>
        constexpr size_t GetComponentIndex()
        {
            return m_componentIndex[std::type_index(typeid(T))];
        }

        template <typename T>
        constexpr bool HasComponent(Entity p_entity)
        {
            return m_entitySignatures[p_entity][GetComponentIndex<T>()];
        }

        constexpr bool IsDead(Entity p_entity) const
        {
            return !m_entitySignatures[p_entity][0];
        }

        template <typename T>
        void ForEachEntityWithComponent(std::function<void(T &)> p_action)
        {
            for (size_t e = 0; e < nextEntityID; e++)
            {
                if (IsDead(e) || !HasComponent<T>(e))
                    continue;

                if (GetComponent<T>(e).has_value())
                {
                    auto &comp = GetComponent<T>(e)->get();
                    p_action(comp);
                }
            }
        }

        std::vector<Entity> QueryEntitiesWithSignature(Signature p_querySignature)
        {
            std::vector<Entity> entities;

            for (Entity e = 0; e < nextEntityID; ++e)
            {
                if (m_entitySignatures[e][0] && (m_entitySignatures[e] & p_querySignature) == p_querySignature)
                {
                    entities.push_back(e);
                }
            }

            return entities;
        }

    private:
        size_t m_nextComponentIndex{ 1 }; // Bit zero reserved for the entitiy state; 1 = alive, 0 = dead
        std::queue<Entity> m_deadEntities{};
        std::array<Signature, MAX_ENTITIES> m_entitySignatures{};
        std::unordered_map<std::type_index, size_t> m_componentIndex{};
        std::unordered_map<std::type_index, std::shared_ptr<void>> m_componentDirectory{};
    };

} // namespace lum

#endif // !ECS_H