#include "wander/ecs/ecs.h"

namespace wander {

Entity World::create() {
    Entity e;
    if (!free_list_.empty()) {
        e = free_list_.back();
        free_list_.pop_back();
    } else {
        e = next_entity_++;
    }
    alive_[e] = true;
    alive_count_++;
    return e;
}

void World::destroy(Entity e) {
    if (!alive(e)) return;

    // Remove from all pools
    for (auto& [key, pool] : pools_) {
        pool->remove(e);
    }

    alive_[e] = false;
    alive_count_--;
    free_list_.push_back(e);
}

bool World::alive(Entity e) const {
    auto it = alive_.find(e);
    return it != alive_.end() && it->second;
}

} // namespace wander
