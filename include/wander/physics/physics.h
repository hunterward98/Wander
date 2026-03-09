#pragma once

#include "wander/core/types.h"
#include <vector>
#include <functional>
#include <unordered_map>

namespace wander {

// Collision layers (bitmask)
using CollisionMask = u32;

// Collider types
enum class ColliderType : u8 {
    AABB,
    Circle
};

// Collider definition
struct Collider {
    u32 entity_id = 0;
    ColliderType type = ColliderType::AABB;
    Rect aabb;              // For AABB colliders
    Vec2 center;            // For circle colliders
    f32 radius = 0.0f;      // For circle colliders
    CollisionMask layer = 1;
    CollisionMask mask = 0xFFFFFFFF;  // Collides with everything by default
    bool is_trigger = false;          // Triggers don't block movement
    bool is_static = false;           // Static colliders don't move
};

// Collision info
struct Collision {
    u32 entity_a;
    u32 entity_b;
    Vec2 normal;       // Collision normal (A -> B)
    f32 depth;         // Penetration depth
    Vec2 contact;      // Contact point
};

// Raycast hit
struct RaycastHit {
    u32 entity_id;
    Vec2 point;
    Vec2 normal;
    f32 distance;
    bool hit;
};

// Trigger callback types
using TriggerCallback = std::function<void(u32 entity_a, u32 entity_b)>;

// Spatial hash grid for broadphase
class PhysicsWorld {
public:
    void init(f32 cell_size = 64.0f);
    void clear();

    // Add/remove colliders
    void add(const Collider& collider);
    void remove(u32 entity_id);
    void update_position(u32 entity_id, Vec2 new_pos);

    // Query
    std::vector<Collision> check_collisions();
    std::vector<u32> query_rect(Rect area, CollisionMask mask = 0xFFFFFFFF);
    std::vector<u32> query_circle(Vec2 center, f32 radius, CollisionMask mask = 0xFFFFFFFF);
    RaycastHit raycast(Vec2 origin, Vec2 direction, f32 max_distance,
                       CollisionMask mask = 0xFFFFFFFF);

    // Move an entity with collision response (slide along walls)
    Vec2 move_and_slide(u32 entity_id, Vec2 velocity, f32 dt);

    // Trigger callbacks
    void on_trigger_enter(TriggerCallback callback);
    void on_trigger_exit(TriggerCallback callback);

    // Process triggers (call after check_collisions)
    void process_triggers(const std::vector<Collision>& collisions);

private:
    using CellKey = i64;
    CellKey cell_key(i32 cx, i32 cy) const;

    f32 cell_size_ = 64.0f;
    std::unordered_map<u32, Collider> colliders_;
    std::unordered_map<CellKey, std::vector<u32>> grid_;

    // Trigger tracking
    std::vector<std::pair<u32, u32>> active_triggers_;
    TriggerCallback on_enter_;
    TriggerCallback on_exit_;

    void insert_to_grid(u32 entity_id, const Collider& col);
    void remove_from_grid(u32 entity_id, const Collider& col);
};

// Standalone collision tests
bool aabb_overlaps(Rect a, Rect b);
bool circle_overlaps(Vec2 c1, f32 r1, Vec2 c2, f32 r2);
bool aabb_circle_overlaps(Rect box, Vec2 center, f32 radius);

} // namespace wander
