#pragma once

#include <cstdint>
#include <limits>
#include <string>

namespace nbody {
using EntityIndex = uint32_t;
using EntityGeneration = uint16_t;
using EntityFlags = uint16_t;

constexpr EntityIndex ENTITY_INDEX_NULL =
    std::numeric_limits<EntityIndex>::max();
constexpr EntityFlags ENTITY_FLAG_FREE = 1;

// Memory layout of Entity
// 32b -> Index into the array
// 16b -> Generation id
// 16b -> flags
class Entity {
   private:
    EntityIndex m_index;
    EntityGeneration m_generation;
    EntityFlags m_flags;

   public:
    Entity(EntityIndex index, EntityGeneration generation, EntityFlags flags)
        : m_index(index), m_generation(generation), m_flags(flags) {}

    EntityIndex index() { return m_index; }
    EntityGeneration generation() { return m_generation; }
    EntityFlags flags() { return m_flags; }

    bool test_flags(EntityFlags flags) { return (m_flags & flags) == flags; }

   public:
    std::string debug_format() {
        return "(nbody::Entity { index: " + std::to_string(m_index) +
               ", generation: " + std::to_string(m_generation) +
               ", flags: " + std::to_string(m_flags) + " })";
    }
};

}  // namespace nbody
