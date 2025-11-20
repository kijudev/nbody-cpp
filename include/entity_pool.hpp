#pragma once

#include <cassert>
#include <vector>

#include "entity.hpp"

namespace nbody {
class EntityPool {
   private:
    std::vector<Entity> m_pool;
    EntityIndex m_next_free_index{ENTITY_INDEX_NULL};

   public:
    Entity insert() {
        if (m_next_free_index == ENTITY_INDEX_NULL) {
            Entity entity(static_cast<EntityIndex>(m_pool.size()), 0, 0);
            m_pool.push_back(entity);
            return entity;
        }

        Entity free_entity = m_pool[m_next_free_index];
        Entity new_entity =
            Entity(m_next_free_index, free_entity.generation() + 1, 0);

        m_next_free_index = free_entity.index();
        m_pool[new_entity.index()] = new_entity;

        return new_entity;
    }

    void destroy(Entity entity) {
        assert(entity.index() != ENTITY_INDEX_NULL);
        assert(entity.index() < m_pool.size());

        Entity destroyed_entity(ENTITY_INDEX_NULL, entity.generation(),
                                ENTITY_FLAG_FREE);

        if (m_next_free_index != ENTITY_INDEX_NULL) {
            Entity& free_entity = m_pool[m_next_free_index];
            free_entity = Entity(entity.index(), free_entity.generation(),
                                 free_entity.flags());
        }

        m_next_free_index = entity.index();
        m_pool[entity.index()] = destroyed_entity;
    }

    Entity get_by_index(EntityIndex index) {
        assert(index != ENTITY_INDEX_NULL);
        assert(index < m_pool.size());

        return m_pool[index];
    }

    bool is_valid(Entity entity) {
        assert(entity.index() != ENTITY_INDEX_NULL);
        assert(entity.index() < m_pool.size());

        Entity pool_entity = m_pool[entity.index()];
        return !pool_entity.test_flags(ENTITY_FLAG_FREE) &&
               entity.generation() == pool_entity.generation();
    }
};
}  // namespace nbody
