#pragma once

#include "wander/core/types.h"
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <memory>

namespace wander {

using Entity = u32;
constexpr Entity INVALID_ENTITY = 0;

// Type-erased component pool
class IComponentPool {
public:
    virtual ~IComponentPool() = default;
    virtual void remove(Entity e) = 0;
    virtual bool has(Entity e) const = 0;
};

template<typename T>
class ComponentPool : public IComponentPool {
public:
    T& add(Entity e, T component) {
        auto idx = data_.size();
        data_.push_back(std::move(component));
        entity_to_index_[e] = idx;
        index_to_entity_.push_back(e);
        return data_.back();
    }

    void remove(Entity e) override {
        auto it = entity_to_index_.find(e);
        if (it == entity_to_index_.end()) return;

        size_t idx = it->second;
        size_t last = data_.size() - 1;

        if (idx < last) {
            data_[idx] = std::move(data_[last]);
            Entity moved_entity = index_to_entity_[last];
            entity_to_index_[moved_entity] = idx;
            index_to_entity_[idx] = moved_entity;
        }

        data_.pop_back();
        index_to_entity_.pop_back();
        entity_to_index_.erase(e);
    }

    bool has(Entity e) const override {
        return entity_to_index_.count(e) > 0;
    }

    T& get(Entity e) {
        return data_[entity_to_index_.at(e)];
    }

    const T& get(Entity e) const {
        return data_[entity_to_index_.at(e)];
    }

    // Iterate all components
    size_t size() const { return data_.size(); }
    T& data_at(size_t i) { return data_[i]; }
    Entity entity_at(size_t i) const { return index_to_entity_[i]; }

private:
    std::vector<T> data_;
    std::unordered_map<Entity, size_t> entity_to_index_;
    std::vector<Entity> index_to_entity_;
};

class World {
public:
    Entity create();
    void destroy(Entity e);
    bool alive(Entity e) const;

    template<typename T>
    T& add(Entity e, T component = {}) {
        auto& pool = get_or_create_pool<T>();
        return pool.add(e, std::move(component));
    }

    template<typename T>
    void remove(Entity e) {
        auto* pool = get_pool<T>();
        if (pool) pool->remove(e);
    }

    template<typename T>
    bool has(Entity e) const {
        auto* pool = get_pool<T>();
        return pool && pool->has(e);
    }

    template<typename T>
    T& get(Entity e) {
        return get_or_create_pool<T>().get(e);
    }

    template<typename T>
    const T& get(Entity e) const {
        return get_pool<T>()->get(e);
    }

    // Iterate entities with specific components
    template<typename... Ts, typename Func>
    void each(Func&& func) {
        // Use the first component pool to drive iteration
        auto* pool = get_pool<std::tuple_element_t<0, std::tuple<Ts...>>>();
        if (!pool) return;

        for (size_t i = 0; i < pool->size(); i++) {
            Entity e = pool->entity_at(i);
            if ((has<Ts>(e) && ...)) {
                func(e, get<Ts>(e)...);
            }
        }
    }

    size_t entity_count() const { return alive_count_; }

private:
    template<typename T>
    ComponentPool<T>& get_or_create_pool() {
        auto key = std::type_index(typeid(T));
        auto it = pools_.find(key);
        if (it == pools_.end()) {
            auto pool = std::make_unique<ComponentPool<T>>();
            auto& ref = *pool;
            pools_[key] = std::move(pool);
            return ref;
        }
        return static_cast<ComponentPool<T>&>(*it->second);
    }

    template<typename T>
    ComponentPool<T>* get_pool() const {
        auto key = std::type_index(typeid(T));
        auto it = pools_.find(key);
        if (it == pools_.end()) return nullptr;
        return static_cast<ComponentPool<T>*>(it->second.get());
    }

    u32 next_entity_ = 1;
    u32 alive_count_ = 0;
    std::vector<Entity> free_list_;
    std::unordered_map<Entity, bool> alive_;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> pools_;
};

} // namespace wander
